#include "storage-driver.h"
#include "../common/common.h"
#include <stdint.h>

// --- Configuration ---
#define VIRTIO_MAGIC_VALUE       0x74726976
#define VIRTIO_ID_BLOCK          2
#define VIRTIO_MMIO_START        0x10001000
#define VIRTIO_MMIO_STEP         0x1000  

// --- MMIO Offsets (Modern V2) ---
#define VIRTIO_MMIO_MAGIC        0x000
#define VIRTIO_MMIO_DEVICE_ID    0x008
#define VIRTIO_MMIO_QUEUE_SEL    0x030
#define VIRTIO_MMIO_QUEUE_NUM    0x038
#define VIRTIO_MMIO_QUEUE_READY  0x044
#define VIRTIO_MMIO_NOTIFY       0x050
#define VIRTIO_MMIO_STATUS       0x070
#define VIRTIO_MMIO_DESC_LOW     0x080
#define VIRTIO_MMIO_DESC_HIGH    0x084
#define VIRTIO_MMIO_AVAIL_LOW    0x090
#define VIRTIO_MMIO_AVAIL_HIGH   0x094
#define VIRTIO_MMIO_USED_LOW     0x0a0
#define VIRTIO_MMIO_USED_HIGH    0x0a4

// --- VirtQueue Structures ---
struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
};

struct virtq_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[16];
};

struct virtq_used_elem {
    uint32_t id;
    uint32_t len;
};

struct virtq_used {
    uint16_t flags;
    uint16_t idx;
    struct virtq_used_elem ring[16];
};

struct virtio_blk_req {
    uint32_t type;
    uint32_t reserved;
    uint64_t sector;
} __attribute__((packed));

// --- Global State ---
__attribute__((aligned(4096)))
uint8_t disk_queue_memory[8192]; // Increased size to safely hold Descriptor+Avail+Used

struct virtq_desc  *desc_table;
struct virtq_avail *avail_ring;
struct virtq_used  *used_ring;
uintptr_t volatile VIRTIO_MMIO_BASE = 0;

uintptr_t find_disk() {
    for (uintptr_t base = VIRTIO_MMIO_START; base <= 0x10008000; base += VIRTIO_MMIO_STEP) {
        uint32_t magic = *(volatile uint32_t*)(base + 0x00);
        uint32_t devid = *(volatile uint32_t*)(base + 0x08);
        
        if (magic == VIRTIO_MAGIC_VALUE && devid == VIRTIO_ID_BLOCK) {
            return base;
        }
    }
    return 0;
}

void init_disk() {
    VIRTIO_MMIO_BASE = find_disk();
    if (VIRTIO_MMIO_BASE == 0) panic("No VirtIO Block Device found.");

    // 1. Reset & Basic Handshake
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_STATUS) = 0;
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_STATUS) |= 0x01; // ACK
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_STATUS) |= 0x02; // DRIVER

    // 2. Feature Negotiation (Simplified: Just tell device we are V1 Modern)
    // there are several features so they have some sort of paging system so you can 'flip' to the next page
    // DeviceFeaturesSel (0x14): This is you telling the Hardware, "Please show me bits 32–63 of the features you support." (Page 1).
    // DriverFeaturesSel (0x24): This is you telling the Hardware, "I am about to write back my acceptance for bits 32–63." (Page 1).
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + 0x14) = 1; // DeviceFeaturesSel = 1 (upper 32)
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + 0x24) = 1; // DriverFeaturesSel = 1
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + 0x20) = (1 << 0); // Accept VIRTIO_F_VERSION_1

    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_STATUS) |= 0x08; // FEATURES_OK
    // NOTE: it seems instantaneous here, but in a real device, you'd probably have to poll or do some interrupt stuff to eventually check if features have been accepted.
    // also although it's called handshake it's more like filling a form
    // more notes: Since you are on a 64-bit RISC-V system, your memory addresses (like 0x80100000) are 64 bits long. However, the VirtIO hardware registers are only 32 bits wide.
    if (!(*(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_STATUS) & 0x08)) panic("Features fail");

    // 3. Queue Setup
    // queues allow different cores to access the disk simultaneously while slots allow processes in some core to access the disk simultaneously. even allowing a process to have more than 1 simutaneous disk request
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_QUEUE_SEL) = 0;
    // 16 is how many slots exist forming some sort of ring.
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_QUEUE_NUM) = 16;

    // flow showing what various rings and tables mean:
    // CPU fills out a Descriptor with a memory address (The "Label").

    // CPU puts that Descriptor's ID into the Available Ring (The "Inbox").

    // CPU kicks the Notify register (The "Doorbell").

    // Disk sees the ID in the Available Ring, looks up the address in the Descriptor Table, and moves the data.

    // Disk puts the ID into the Used Ring (The "Outbox") to say "Done!"

    // CPU sees the update in the Used Ring and tells the process, "Your data is ready."
    // --- Pointer Math for Global Hooks ---
    uintptr_t addr = (uintptr_t)disk_queue_memory;
    desc_table = (struct virtq_desc *)addr;
    avail_ring = (struct virtq_avail *)(addr + (16 * sizeof(struct virtq_desc)));
    
    // Page align the used ring (standard requirement for many devices)
    uintptr_t used_addr = ( (uintptr_t)avail_ring + sizeof(struct virtq_avail) + 4095) & ~4095;
    used_ring = (struct virtq_used *)used_addr;

    // Write physical addresses to hardware
    // DESC table
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_DESC_LOW)  = (uint32_t)addr;
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_DESC_HIGH) = (uint32_t)(addr >> 32);
    
    // AVAIL ring
    // Note: Cast to uintptr_t FIRST, then to uint32_t to satisfy the compiler
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_AVAIL_LOW) = (uint32_t)(uintptr_t)avail_ring;
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_AVAIL_HIGH)= (uint32_t)((uintptr_t)avail_ring >> 32);

    // USED ring
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_USED_LOW)  = (uint32_t)(uintptr_t)used_ring;
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_USED_HIGH) = (uint32_t)((uintptr_t)used_ring >> 32);

    // 4. Finalize
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_QUEUE_READY) = 1;
    __sync_synchronize(); 
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_STATUS) |= 0x04; // DRIVER_OK
}

void disk_read(uint64_t sector_number, void *buffer) {
    static struct virtio_blk_req header;
    static volatile uint8_t status; 

    header.type = 0; // Read
    header.sector = sector_number;
    status = 0xff;

    // 1. Descriptor Chain Setup
    // what to do? flags simply say I'm not done and go to next descriptor with .next value.
    desc_table[0].addr = (uintptr_t)&header;
    desc_table[0].len = sizeof(header);
    desc_table[0].flags = 1; // NEXT
    desc_table[0].next = 1;

    // where to access/put the data
    desc_table[1].addr = (uintptr_t)buffer;
    desc_table[1].len = 512;
    desc_table[1].flags = 1 | 2; // NEXT | WRITE (for device)
    desc_table[1].next = 2;

    // did the task succeed
    desc_table[2].addr = (uintptr_t)&status;
    desc_table[2].len = 1;
    desc_table[2].flags = 2; // WRITE
    desc_table[2].next = 0;

    // NOTE: it's a chain because these pieces of information might be spread out(it feels legacy though)

    // 2. Ring Notification
    // the avail_ring is just a list of Descriptor Table indices. By writing 0, you are telling the disk: "Start your journey at Descriptor Slot 0."
    avail_ring->ring[avail_ring->idx % 16] = 0; 
    __asm__ volatile ("fence rw, rw");
    avail_ring->idx++; 
    // avail_ring->idx++ is how the disk knows how many new tasks there are. It compares its own last-seen index to this number
    __asm__ volatile ("fence rw, rw");

    // Notify hardware of Queue 0
    // You are writing 0 because you are notifying the hardware about Queue 0. If you were using Queue 1, you would write a 1 here.
    *(volatile uint32_t*)(VIRTIO_MMIO_BASE + VIRTIO_MMIO_NOTIFY) = 0; 

    // 3. Wait for Hardware
    // a better wait loop would be one with a timeout leading to shutdown
    while(status == 0xff) {
        __asm__ volatile ("nop"); 
    }
}