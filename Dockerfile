FROM debian:buster-slim as patcher

RUN apt-get -y update \
    && apt-get -y --no-install-recommends install unzip patch \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /root/spl
WORKDIR /root/spl

ADD https://raw.githubusercontent.com/gicking/STM8-SPL_SDCC_patch/master/STM8L10x_StdPeriph_Lib_V1.2.1_sdcc.patch .
ADD https://raw.githubusercontent.com/gicking/STM8-SPL_SDCC_patch/master/STM8L15x-16x-05x-AL31-L_StdPeriph_Lib_V1.6.2_sdcc.patch .
ADD https://raw.githubusercontent.com/gicking/STM8-SPL_SDCC_patch/master/STM8S_StdPeriph_Lib_V2.3.1_sdcc.patch .
ADD https://raw.githubusercontent.com/gicking/STM8-SPL_SDCC_patch/master/STM8TL5x_StdPeriph_Lib_V1.0.1.patch .

COPY spl/en.stsw-stm8012.zip  spl/en.stsw-stm8016.zip  spl/en.stsw-stm8030.zip  spl/en.stsw-stm8069.zip ./
RUN unzip en.stsw-stm8012.zip \
  && unzip en.stsw-stm8016.zip \
  && unzip en.stsw-stm8030.zip \
  && unzip en.stsw-stm8069.zip \
  && patch -p0 < STM8L10x_StdPeriph_Lib_V1.2.1_sdcc.patch \
  && patch -p0 < STM8L15x-16x-05x-AL31-L_StdPeriph_Lib_V1.6.2_sdcc.patch \
  && patch -p0 < STM8S_StdPeriph_Lib_V2.3.1_sdcc.patch \
  && patch -p0 < STM8TL5x_StdPeriph_Lib_V1.0.1.patch

FROM debian:buster-slim

RUN apt-get -y update \
  && apt-get -y --no-install-recommends install sdcc make \
  && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /root/spl
COPY --from=patcher /root/spl/STM8L10x_StdPeriph_Lib/Libraries/STM8L10x_StdPeriph_Driver /root/spl/STM8L10x_StdPeriph_Driver
COPY --from=patcher /root/spl/STM8L15x-16x-05x-AL31-L_StdPeriph_Lib/Libraries/STM8L15x_StdPeriph_Driver /root/spl/STM8L15x_StdPeriph_Driver
COPY --from=patcher /root/spl/STM8TL5x_StdPeriph_Lib_V1.0.1/Libraries/STM8TL5x_StdPeriph_Driver /root/spl/STM8TL5x_StdPeriph_Driver
COPY --from=patcher /root/spl/STM8S_StdPeriph_Lib/Libraries/STM8S_StdPeriph_Driver /root/spl/STM8S_StdPeriph_Driver

VOLUME /root/workspace
WORKDIR /root/workspace
