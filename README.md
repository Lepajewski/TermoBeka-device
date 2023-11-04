# TermoBeka-device

## Installation Guide

### Windows 10

1. Install [Docker Desktop](https://www.docker.com/products/docker-desktop/). Do not check *Enable integration with my default WSL distro* in settings.

2. Install [Remote Development](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack) extenstion in VS Code. Do not check *Execute In WSL* in DEV Container: settings

3. Install [Ubuntu 20.04 LTS](https://apps.microsoft.com/store/detail/ubuntu-20046-lts/9MTTCL66CPXJ)
> Note: kernel version must be higher than `5.10.60.1`, check by `uname -a` on Ubuntu or by `wsl --version` on CMD

4. Make Ubuntu default WSL in CMD: `wsl -l -v` and `wsl --set-default Ubuntu-20.04`

5. Install usbipd in CMD: `winget install usbipd`

6. In Ubuntu run commands:
- `sudo apt-get update && sudo apt-get -qqy upgrade`
- `sudo apt install linux-tools-virtual hwdata`
- `sudo update-alternatives --install /usr/local/bin/usbip usbip $(command -v ls /usr/lib/linux-tools/*/usbip | tail -n1) 20`
- `sudo udevadm control --reload`
- `sudo service udev restart`

7. Plug board to computer (on devkit use UART MicroUSB port).

8. In CMD run:
- `usbipd wsl list`
- `usbipd wsl attach --busid id-id`

Now device should be listed as attached:
- `usbipd wsl list`

> Note: more advanced instructions for steps 6-8 available [here](https://github.com/dorssel/usbipd-win/wiki/WSL-support)

9. Open folder in Dev Container:
- first use: `F1` + `Dev Containers: Rebuild Without Cache and Reopen in Container`
- further use: `F1` + `Dev Containers: Reopen in Container`

10. Wait for PlatrofmIO to setup. Reload window after setup.

11. Finally run in terminal:
- `make all`

