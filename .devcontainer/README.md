# EGSnrc Development Container

Development Containers is an open standard that is supported by a number of 
[development tools](https://containers.dev/supporting). A supporting tool allows
you to develop from your local IDE, such as VS Code and connect to a "remote" 
development container that hosts the tools and applications for your code.
In this case, EGSnrc builds and runs in a Linux container but you can be working on a Windows
or MacOS Desktop.

## Supported Environments

The following environments are all known to work, but any tool that support the development container
specification should work.

### GitHub Codespaces

This is a paid feature, but if you have access to it then running EGSnrc is as simple as clicking
the **Code** button in GitHub and choosing to open your repository in GitHub Codespaces. Codespaces
runs VS Code right inside your web browser but you also have the option of using Codespaces from
a desktop installation of VS Code. When you are using Codespaces, the remote development container runs
in the cloud and is hosted by GitHub.

### VS Code

This requires [VS Code](https://code.visualstudio.com/) on Windows or MacOS with
[Docker Desktop](https://www.docker.com/products/docker-desktop) installed or VS Code on
Linux with [Docker CE/EE](https://docs.docker.com/install/#supported-platforms). You also need
the VS Code [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
installed. When using these options, the development container runs on your local machine using Docker.

To use this option, once the above requirements are installed, you simply clone your Git repository and
open it in VS Code. It will detect that there is a devcontainer configuration and ask if you want to open
your folder in the development container. Say yes and this will build and launch a docker container that
is running the code you have cloned.

### Windows Notes

When running locally on Windows using Docker there is an issue with line-endings in files. When you clone
the repository by default git will convert the line endings in all files to CRLF which is the windows standard.
Since we are going to be running the code on Linux inside a container, we need the files to have the Unix
standard LF line endings. There are a couple of ways to handle this:

1. Run these commands before you clone your repository so that git does not convert line endings and treats
LF as the default.

```
git config --global core.eol lf
git config --global core.autocrlf input
```

These are global settings and if you work on other Git repositories you may not want to do this. But
if you only are using git for this one repository then this is the easy way to do it.

2. Enable these settings when you clone the repository:

```
git clone -c core.autocrlf=false -c core.eol=lf https://github.com/nrc-cnrc/EGSnrc
```

3. Fix the settings and the line endings in all the files after cloning the repository. Run these from the
root of the cloned repository. If you have made any changes you will lose them, so save them somewhere before
running these commands.

```
git config core.eol lf
git config core.autocrlf input
git add --update --renormalize
git rm -rf --cached .
git reset --hard HEAD
```

## Runtime Experience

Regardless of which option you use, when the development container starts for the first time it
needs to download the base Docker image, install the EGSnrc-specific required packages, and then
configure and build EGSnrc for the first time. Subsequent starts of the development container will
skip these steps and start faster. Once the container is running you use and run EGSrnc the same
as you would on any platform. In other words, just follow the standard documentation.

The development container configuration includes building and running the GUI apps. The container
configuration installs a lightweight GUI desktop that you can access in your browser. Access to
this desktop will open in your browser automatically when you first start the container but you can open
it at any time from the **Ports** tab in VS Code. You can run commands like `egs_view` that launch
GUI applications from the VS Code terminal and they will appear in the UI in the lightweight desktop
once you open it. You can also run a terminal inside the GUI desktop if preferred.

The default password to login to the GUI is `vscode`.

## Reference Information

* https://containers.dev/
* [VSCode: Developing inside a Container](https://code.visualstudio.com/docs/devcontainers/containers)
