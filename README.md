# OS Contract Synthesis

## Usage
- Clone the repo and run `docker build -t contract-generator .`
- Then run `docker run -it contract-generator` to generate contracts.
- If you don't want to use docker, see the docker file and use the instructions in there to prepare the environment and run the java main class(ContractGen.java).

## How it works
- The os is compiled without any user programs at the beginning. Two compiled versions of the os exist. One for cpu simulation and the other for spike simulation. The version for spike simulation ensures that only one user program can be run on the cpu while the version for cpu allows for two. These compiled oses are used for simulation throughout the program.
- The test cases are generated randomly using the methodology specified in the original paper. A test case is however defined as two programs. One representing an attacker process and the other a victim process(it hasn't fully been decided what an attacker should look like(see src/main/resources/os/program-templates/ for more details). We also considers syscall-level observations. A syscall-level observation may leak the syscall type or one of the syscall argument(based on the linux riscv abi) and considers the entire syscall. For now syscalls are generated in blocks where all the arguments are specified before each syscall is executed(see src/main/java/contractgen/riscv/isa/tests/RISCVTestGenerator.java for more details. In particular check the alterObservations method)
- For each simulation, each of the two test cases are first compiled into the os and executed on a verilator simulator of some target cpu(in this case ibex). The simulator executes each os-testcase binary in parallel on a cpu emulator and when the cpu reaches the attacker process, compares the architectural state of the cpu for both test cases(see src/main/resources/os/simulators/sourcefiles/ibex/ibexos_simple_system_main.cc). The list of signals which are checked during each cycle is contained in rvfi_signals.hpp. In particular note rvfi_ext_mcycle.
- After determining attacker distinguishability, the next step is to extract contract observations. This is done via spike. We use the spike compiled os and then attach only the victim process to it for each test case. Then we run spike on each of these os-testcase binaries and for each instruction, we extract any observational differences(see RVFIOSExtractor.java for more details. this file closely follows RVFIExtractor.java adapting it as required)
- Finally the rest of the program runs as detailed in the original paper and the ilp solver eventually generates contracts based on attacker distinguishability and the extracted contract observations.

## Roadmap
- Determine shape of attacker process to generate better contracts and think about test case generation generally. 
- Currently, it appears the generated contracts do not contain syscall observations. Fix that.
- Add more syscalls
- Repeat simulation process for another cpu(cva6)
- Fix bugs as they pop up
- Remove a few hardcoded constants(such as those in ibexos_simple_system_main.cc)

## OS Features
- Runs on both rv32 and rv64 cores
- Can execute small riscv programs and make simple syscalls. Currently supported syscalls are write, exit and yield
- Uses a round robin scheduler to schedule programs for given time slices.
- Protects processes from each other using pmp.
