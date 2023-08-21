FROM mcr.microsoft.com/vscode/devcontainers/base:ubuntu

RUN apt-get update \
    && apt-get -qqy install \
    bc \
    sudo \
    build-essential \
    ca-certificates \
    clang \
    curl \
    gcc \
    git \
    python3 \
    python3-dev \
    python3-distutils \
    python3-pip \
    python3-setuptools \
    python3-venv \
    srecord \
    udev \
    xz-utils \
    usbutils \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fLo /etc/udev/rules.d/99-platformio-udev.rules --create-dirs https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules

USER $USERNAME

RUN python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py)"

CMD ["sleep", "infinity"]


# FROM mcr.microsoft.com/vscode/devcontainers/base:ubuntu

# ENV USERNAME=vscode

# RUN apt-get update \
#     && apt-get -qqy install \
#     bc \
#     sudo \
#     build-essential \
#     ca-certificates \
#     clang \
#     curl \
#     gcc \
#     git \
#     python3 \
#     python3-dev \
#     python3-distutils \
#     python3-pip \
#     python3-setuptools \
#     python3-venv \
#     srecord \
#     udev \
#     xz-utils \
#     && apt-get autoremove -y \
#     && apt-get clean -y \
#     && rm -rf /var/lib/apt/lists/*

# RUN curl -fLo /etc/udev/rules.d/99-platformio-udev.rules --create-dirs https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules


# # RUN chown $USERNAME /dev/ttyUSB0

# USER $USERNAME

# # RUN sudo apt-get update \
# #     && apt-get -qqy install \
# #     linux-tools-generic \
# #     hwdata

# # RUN sudo update-alternatives --install /usr/local/bin/usbip usbip $(command -v ls /usr/lib/linux-tools/*/usbip | tail -n1) 20 
# # RUN sudo udevadm control --reload

# # RUN sudo usermod -aG plugdev $USERNAME
# # RUN sudo usermod -aG dialout $USERNAME

# RUN python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py)"

# CMD ["sleep", "infinity"]
