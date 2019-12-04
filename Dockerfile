FROM debian:buster-slim as patcher

RUN apt-get -y update \
    && apt-get -y --no-install-recommends install unzip patch \
    && rm -rf /var/lib/apt/lists/*


RUN mkdir -p /root/spl
WORKDIR /root/spl

COPY spl/en.stsw-stm8012.zip  spl/en.stsw-stm8016.zip  spl/en.stsw-stm8030.zip  spl/en.stsw-stm8069.zip .


# FROM debian:buster-slim
#
# WORKDIR /root
# RUN mkdir -p spl/STM8S_StdPeriph_Lib/inc spl/STM8S_StdPeriph_Lib/src
# COPY spl/STM8S_StdPeriph_Lib/Libraries/STM8S_StdPeriph_Driver/inc spl/STM8S_StdPeriph_Lib/inc
# COPY spl/STM8S_StdPeriph_Lib/Libraries/STM8S_StdPeriph_Driver/src spl/STM8S_StdPeriph_Lib/src
#
# RUN apt-get -y update && apt-get -y --no-install-recommends install sdcc make && rm -rf /var/lib/apt/lists/*
#
# VOLUME /root/workspace
# WORKDIR /root/workspace
