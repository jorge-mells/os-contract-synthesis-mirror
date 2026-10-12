FROM python:3.11-slim-bookworm AS python

FROM verilator/verilator:v5.046 AS verilator

COPY --from=python /usr/local /usr/local

ENV DEBIAN_FRONTEND=noninteractive

# TODO: pin versions
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        curl=8.5.0-2ubuntu10.15 \
        libelf-dev=0.190-1.1ubuntu0.1 \
        device-tree-compiler \
        libboost-regex-dev \
        libboost-system-dev \
        build-essential && \
    rm -rf /var/lib/apt/lists/*

ENV RISCV=/opt/ibex/riscv
ENV PATH=$RISCV/bin:$PATH

WORKDIR /spike

# TODO: pin commit
RUN git clone https://github.com/riscv-software-src/riscv-isa-sim.git .

RUN mkdir build && \
    cd build && \
    ../configure --prefix="$RISCV" && \
    make -j"$(nproc)" && \
    make install && \
    cd .. && \
    rm -rf build 

WORKDIR /ibex

ARG BUILDARCH

RUN if [ "$BUILDARCH" = "amd64" ]; then \
        echo "download x86_64 toolchain"; \
        curl -L \
          -o /tmp/toolchain.tar.xz \
          'https://github.com/lowRISC/lowrisc-toolchains/releases/download/20250710-1/lowrisc-toolchain-gcc-rv64imac-x86_64-20250710-1.tar.xz'; \
    elif [ "$BUILDARCH" = "arm64" ]; then \
        echo "download ARM64 toolchain"; \
        curl -L \
          -o /tmp/toolchain.tar.xz \
          'https://github.com/lowRISC/lowrisc-toolchains/releases/download/20250710-1/lowrisc-toolchain-gcc-rv64imac-aarch64-20250710-1.tar.xz'; \
    fi

RUN mkdir -p /opt/ibex/riscv \
    && tar -xJ \
        --strip-components=1 \
        -f /tmp/toolchain.tar.xz \
        -C /opt/ibex/riscv \
    && rm /tmp/toolchain.tar.xz


RUN git clone https://github.com/lowRISC/ibex.git . && git checkout 3250d994

RUN pip install --no-cache-dir -U -r python-requirements.txt

WORKDIR /os-contract-synthesis

# TODO: pin commit
RUN git clone https://github.com/jorge-mells/os-contract-synthesis-mirror . && git checkout 44981b9

RUN cp -rf /os-contract-synthesis/src/main/resources/os/simulators/sourcefiles/ibex/. /ibex/examples/simple_system/

WORKDIR /ibex

RUN fusesoc --cores-root=. run --target=sim --setup --build \
        lowrisc:ibex:ibexos_simple_system $(util/ibex_config.py small fusesoc_opts) --PMPEnable=1

RUN cp -f /ibex/build/lowrisc_ibex_ibexos_simple_system_0/sim-verilator/Vibex_simple_system /os-contract-synthesis/src/main/resources/os/simulators/Vibex1

FROM maven:3-eclipse-temurin-21

ENV RISCV=/opt/ibex/riscv
ENV PATH=$RISCV/bin:$PATH

COPY --from=verilator /os-contract-synthesis /os-contract-synthesis
COPY --from=verilator /opt/ibex /opt/ibex

RUN apt-get update && apt-get install -y make device-tree-compiler libboost-system-dev libboost-regex-dev

WORKDIR /os-contract-synthesis

# change number of cores and number of tests with these bash instructions
# change cores from 2 to 8
RUN sed -i 's/2,/8,/' src/main/java/contractgen/CONFIG.java 
# change number of tests(for both training and evaluation) from 100 to 1000
RUN sed -i 's/100,/1000,/' src/main/java/contractgen/CONFIG.java 
RUN mvn package


ENTRYPOINT []
#
# CMD ["/bin/bash"]
CMD ["java", "-cp", "target/contractgen-1.0-SNAPSHOT.jar", "src/main/java/contractgen/ContractGen.java"]
