FROM wolfeidau/esp8266-dev:1.4.0

# add all the SDK stuff to the PATH
ENV PATH=$PATH:/opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin

# Path which contains your esp8266 project source code
WORKDIR /Users/luca/code/esp8266-dht22

# pass -v /Users:/Users to ensure your shared folder is available within 
# the container for builds.
VOLUME /Users

USER ubuntu
