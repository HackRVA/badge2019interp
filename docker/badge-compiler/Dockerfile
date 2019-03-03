FROM debian:buster

MAINTAINER Chris Fulara <fularac@umich.edu>

ENV DEBIAN_FRONTEND noninteractive

# Microchip tools require i386 compatability libs
RUN dpkg --add-architecture i386 \
    && apt-get update -yq \
    && apt-get install -yq --no-install-recommends curl build-essential \
    libc6:i386 libstdc++6:i386

# Download and install XC32 compiler
RUN curl -fSL -A "Mozilla/4.0" -o /tmp/xc32.run "http://ww1.microchip.com/downloads/en/DeviceDoc/xc32-v1.34-full-install-linux-installer.run" \
    && chmod a+x /tmp/xc32.run \
    && /tmp/xc32.run --mode unattended --unattendedmodeui none \
        --netservername localhost \
    && rm /tmp/xc32.run

ENV PATH /opt/microchip/xc32/v1.34/bin/:$PATH

USER build
