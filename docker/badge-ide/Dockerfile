FROM badge-compiler

MAINTAINER Chris Fulara <fularac@umich.edu>
USER root
ENV DEBIAN_FRONTEND noninteractive
ENV MPLABX_VERSION 5.10

# Microchip compatability libs
RUN apt-get install -yq --no-install-recommends \
    libx11-6:i386 libxext6:i386 libexpat1:i386 \
    libxext6 libxrender1 libxtst6 libgtk2.0-0 libxslt1.1 procps

# Download and install MPLAB X IDE
# Use url: http://www.microchip.com/mplabx-ide-linux-installer to get the latest version
RUN curl -fSL -A "Mozilla/4.0" -o /tmp/mplabx-installer.tar "http://ww1.microchip.com/downloads/en/DeviceDoc/MPLABX-v${MPLABX_VERSION}-linux-installer.tar" \
    && tar xf /tmp/mplabx-installer.tar && rm /tmp/mplabx-installer.tar \
    && USER=root ./MPLABX-v${MPLABX_VERSION}-linux-installer.sh --nox11 \
        -- --unattendedmodeui none --mode unattended \
    && rm ./MPLABX-v${MPLABX_VERSION}-linux-installer.sh

VOLUME ["/tmp/.X11-unix"]
ENV HOME /home/build

RUN useradd -d /home/build -m -s /bin/sh build \
  && chown -R build:build /home/build

CMD ["/usr/bin/mplab_ide"]
USER build
