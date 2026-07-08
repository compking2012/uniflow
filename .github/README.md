<picture>
  <source media="(prefers-color-scheme: dark)" srcset="https://github.com/uniflow/uniflow-artwork/blob/main/logo/uniflow-logo-dark-200px.png?raw=true">
  <source media="(prefers-color-scheme: light)" srcset="https://github.com/uniflow/uniflow-artwork/blob/main/logo/uniflow-logo-light-200px.png?raw=true">
  <img alt="Uniflow" src="https://github.com/user-attachments/assets/f005b958-24df-4f4a-9bfd-4f834dae59d6">
</picture>

**Uniflow** is a free and open source keyboard and mouse sharing app.
Use the keyboard, mouse, or trackpad of one computer to control nearby computers,
and work seamlessly between them.
It's like a software KVM (but without the video).
TLS encryption is enabled by default. Wayland is supported. Clipboard sharing is supported.

Uniflow is based on Deskflow, with optimizations and enhancements for the Uniflow distribution.

> [!TIP]
>
> **Chat with us**
>
> - Main discussion on Matrix: [`#uniflow:matrix.org`](https://matrix.to/#/#uniflow:matrix.org) ([Matrix clients](https://matrix.org/ecosystem/clients/))
> - Discussion also happens on IRC: `#uniflow` or `#uniflow-dev` on [Libera Chat](https://libera.chat/)
> - Start a [new discussion](https://github.com/uniflow/uniflow/discussions) on our GitHub project.

## Download

[![Downloads: Stable Release](https://img.shields.io/github/downloads/uniflow/uniflow/latest/total?style=for-the-badge&logo=github&label=Download%20Stable)](https://github.com/uniflow/uniflow/releases/latest)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[![Downloads: Continuous Build](https://img.shields.io/github/downloads/uniflow/uniflow/continuous/total?style=for-the-badge&logo=github&label=Download%20Continuous)](https://github.com/uniflow/uniflow/releases/continuous)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[![Download From Flathub](https://img.shields.io/flathub/downloads/org.uniflow.uniflow?style=for-the-badge&logo=flathub&label=Download%20from%20flathub)](https://flathub.org/apps/org.uniflow.uniflow)

> [!NOTE]
> On Windows, you will need to install the
> [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-microsoft-visual-c-redistributable-version).  
> Download latest: [`vc_redist.x64.exe`](https://aka.ms/vc14/vc_redist.x64.exe) [`vc_redist.arm64.exe`](https://aka.ms/vc14/vc_redist.arm64.exe)

> [!TIP]
> For macOS users, the easiest way to install and stay up to date is to use [Homebrew](https://brew.sh) with our [homebrew-tap](https://github.com/uniflow/homebrew-tap).
> macOS reports unsigned apps as damaged. This occurs because we do not use an Apple certificate for notarization. Clear the quarantine attribute to run the app: `xattr -c Uniflow.app`

To use Uniflow, download one of our [packages](https://github.com/uniflow/uniflow/releases), install `uniflow` (from your package repository), or [build it](https://github.com/uniflow/uniflow/wiki/Building) from source.

## Stats

[![GitHub commit activity](https://img.shields.io/github/commit-activity/m/uniflow/uniflow?logo=github)](https://github.com/uniflow/uniflow/commits/master/)
[![GitHub top language](https://img.shields.io/github/languages/top/uniflow/uniflow?logo=github)](https://github.com/uniflow/uniflow/commits/master/)
[![GitHub License](https://img.shields.io/github/license/uniflow/uniflow?logo=github)](LICENSE)
[![REUSE status](https://api.reuse.software/badge/github.com/uniflow/uniflow)](https://api.reuse.software/info/github.com/uniflow/uniflow)

[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=uniflow_uniflow&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=uniflow_uniflow)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=uniflow_uniflow&metric=coverage)](https://sonarcloud.io/summary/new_code?id=uniflow_uniflow)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=uniflow_uniflow&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=uniflow_uniflow)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=uniflow_uniflow&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=uniflow_uniflow)

[![CI](https://github.com/uniflow/uniflow/actions/workflows/continuous-integration.yml/badge.svg)](https://github.com/uniflow/uniflow/actions/workflows/continuous-integration.yml)
[![CodeQL Analysis](https://github.com/uniflow/uniflow/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/uniflow/uniflow/actions/workflows/codeql-analysis.yml)
[![SonarCloud Analysis](https://github.com/uniflow/uniflow/actions/workflows/sonarcloud-analysis.yml/badge.svg)](https://github.com/uniflow/uniflow/actions/workflows/sonarcloud-analysis.yml)

## Contribute

[![Good first issues](https://img.shields.io/github/issues/uniflow/uniflow/good%20first%20issue?label=good%20first%20issues&color=%2344cc11)](https://github.com/uniflow/uniflow/labels/good%20first%20issue)

There are many ways to contribute to the Uniflow project.

We're a friendly, active, and welcoming community focused on building a great app.

Read our [Contributing](https://github.com/uniflow/uniflow/wiki/Contributing) page to get started.

For instructions on building Uniflow, use the wiki page: [Building](https://github.com/uniflow/uniflow/wiki/Building)

## Operating Systems

We support all major operating systems, including Windows, macOS, Linux, and Unix-like BSD-derived.

Windows 10 v1809 or higher is required.

macOS 13 or higher is required to use our CI builds for Apple Silicon machines. macOS 12 or higher is required for Intel macs or local builds.

Linux requires libei 1.3+ and libportal 0.8+ for the server/client. Additionally, Qt 6.7+ is required for the GUI.
Linux users with systems not meeting these requirements should use flatpak in place of a native package.

We officially support FreeBSD, and would also like to support: OpenBSD, NetBSD, DragonFly, Solaris.

## Repology

Repology monitors a huge number of package repositories and other sources comparing package
versions across them and gathering other information.

[![Repology](https://repology.org/badge/vertical-allrepos/uniflow.svg?columns=2&exclude_unsupported)](https://repology.org/project/uniflow/versions)

## Installing on macOS

When you install Uniflow on macOS, you need to allow accessibility access (Privacy & Security) to both the `Uniflow` app and the `uniflow` process.

If using Sequoia, you may also need to allow `Uniflow` under Local Network‍ settings (Privacy & Security).
When prompted by the OS, go to the settings and enable the access.

If you are upgrading and you already have `Uniflow` or `uniflow`
on the allowed list you will need to manually remove them before accessibility access can be granted to the new version.

macOS users who download directly from releases may need to run `xattr -c /Applications/Uniflow.app` after copying the app to the `Applications` dir.

It is recommended to install Uniflow using [Homebrew](https://brew.sh) from our [homebrew-tap](https://github.com/uniflow/homebrew-tap)

To add our tap, run:

```
brew tap uniflow/tap
```

Then install either:

- Stable: `brew install uniflow`
- Continuous: `brew install uniflow-dev`

## Similar Projects

In the open source developer community, similar projects collaborate for the improvement of all
mouse and keyboard sharing tools. We aim for idea sharing and interoperability.

- [**Lan Mouse**](https://github.com/feschber/lan-mouse) -
  Rust implementation with the goal of having native front-ends and interoperability with
  Uniflow/Synergy.
- [**Synergy**](https://symless.com/synergy) -
  Downstream commercial fork. Synergy sponsors Uniflow with financial support and contributes code ([learn more](https://github.com/uniflow/uniflow/wiki/Relationship-with-Synergy)).
- [**Input Leap**](https://github.com/input-leap/input-leap) -
  Inactive Uniflow/Synergy-derivative with the goal continuing Barrier development (now a dead fork).

## FAQ

### Is Uniflow compatible with Synergy, Input Leap, or Barrier?

Yes, Uniflow has network compatibility with all forks:

- Requires Uniflow >= v1.17.0.96
- Uniflow will _just work_ with Input Leap and Barrier (server or client).
- Connecting a Uniflow client to a Synergy 1 server will also _just work_.
- To connect a Synergy 1 client, you need to select the Synergy protocol in the Uniflow server settings.

_Note:_ Only Synergy 1 is compatible with Uniflow (Synergy 3 is not yet compatible).

### Is Uniflow compatible with Lan Mouse?

We would love to see compatibility with Lan Mouse. This may be quite an effort as currently the way they handle the generated input is very different.

### If I want to solve issues in Uniflow do I need to contribute to a fork?

We welcome PRs (pull requests) from the community. If you'd like to make a change, please feel
free to [start a discussion](https://github.com/uniflow/uniflow/discussions) or
[open a PR](https://github.com/uniflow/uniflow/wiki/Contributing).

### Is clipboard sharing supported?

Absolutely. The clipboard-sharing feature is a cornerstone feature of the product and we are
committed to maintaining and improving that feature.

### Is Wayland for Linux supported?

Yes! Wayland (the Linux display server protocol aimed to become the successor of the X Window
System) is an important platform for us.
The [`libei`](https://gitlab.freedesktop.org/libinput/libei) and
[`libportal`](https://github.com/flatpak/libportal) libraries enable
Wayland support for Uniflow. We would like to give special thanks to Peter Hutterer,
who is the author of `libei`, a major contributor to `libportal`, and the author of the Wayland
implementation in Uniflow. Others such as Olivier Fourdan and Povilas Kanapickas helped with the
Wayland implementation.

Some features _may_ be unavailable or broken on Wayland. Please see the [known Wayland issues](https://github.com/uniflow/uniflow/discussions/7499).

### Where did it all start?

The upstream Deskflow lineage began as Synergy in 2001 by Chris Schoeneman.
Read about the [history of the project](https://github.com/uniflow/uniflow/wiki/History) on our wiki.

## Meow'Dib (our mascot)

![Meow'Dib](https://github.com/user-attachments/assets/726f695c-3dfb-4abd-875d-ed658f6c610f)

## Uniflow Contributors

[![Sponsored by Synergy](https://raw.githubusercontent.com/uniflow/uniflow-artwork/b2c72a3e60a42dee793bd47efc275b5ee0bdaa5f/misc/synergy-sponsor.svg)](https://symless.com/synergy)

[Synergy](https://symless.com/synergy) sponsors the Uniflow project by contributing code and providing financial support ([learn more](https://github.com/uniflow/uniflow/wiki/Relationship-with-Synergy)).

Uniflow is made by possible by these contributors.

 <a href = "https://github.com/uniflow/uniflow/graphs/contributors">
   <img src = "https://contrib.rocks/image?repo=uniflow/uniflow"/>
 </a>

## License

This project is licensed under [GPL-2.0](LICENSE) with an [OpenSSL exception](../LICENSES/LicenseRef-OpenSSL-Exception.txt).
