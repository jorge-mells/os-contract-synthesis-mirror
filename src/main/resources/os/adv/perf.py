import pylibfst
import time
import sys

def no_op_callback(data, time, handle, value):
    """
    The 'No-Op' callback. 
    It does nothing, returning immediately.
    """
    pass

def run_noop_test(fst_path):
    print(f"Opening trace: {fst_path}")
    
    # Open the reader
    fst = pylibfst.lib.fstReaderOpen(fst_path.encode("UTF-8"))
    
    # Enable all signals to ensure we are testing the maximum callback frequency
    pylibfst.lib.fstReaderClrFacProcessMaskAll(fst)
    pylibfst.lib.fstReaderSetFacProcessMaskAll(fst)
    
    print("Starting iteration with No-Op callback...")
    start_time = time.time()
    
    # Run the iterator
    pylibfst.helpers.fstReaderIterBlocks(fst, no_op_callback, None)
    
    end_time = time.time()
    
    # Cleanup
    pylibfst.lib.fstReaderClose(fst)
    
    duration = end_time - start_time
    print("-" * 30)
    print(f"Test complete.")
    print(f"Total time: {duration:.4f} seconds")
    
    if duration > 5:
        print("Verdict: The Python-to-C context switch overhead is significant.")
    else:
        print("Verdict: Callback overhead is low; bottleneck is likely your logic.")
    print("-" * 30)

if __name__ == "__main__":
    # Replace with your actual FST file path
    FST_FILE = "sim2.fst"
    
    try:
        run_noop_test(FST_FILE)
    except Exception as e:
        print(f"Error running test: {e}")