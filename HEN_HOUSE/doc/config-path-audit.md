# EGSnrc configuration path audit

Reference for [issue #1211](https://github.com/nrc-cnrc/EGSnrc/issues/1211):
migrate user session config and user-generated `specs/*.conf` to
`$XDG_CONFIG_HOME/EGSnrc/` (default `~/.config/EGSnrc/`).

## Layout

| Path | Tag | Role |
|------|-----|------|
| `HEN_HOUSE/specs/*.spec`, `*_example.conf` | install-tree | Shared makefile fragments; stay in install |
| `~/.config/EGSnrc/profiles/<name>/specs/*.conf` | user-spec | Machine configs from `configure` |
| `~/.config/EGSnrc/profiles/<name>/env` | user-session | `EGS_HOME`, `EGS_CONFIG` exports |
| `~/.config/EGSnrc/profiles/<name>/hen_house` | user-session | Install root for profile |
| `~/.config/EGSnrc/profiles/<name>/active_conf` | user-session | Basename of active machine config |
| `~/.config/EGSnrc/active_profile` | user-session | Currently selected profile name |
| `~/.config/EGSnrc/EGSnrc.bash` | user-session | Shell entry (one RC line) |
| `~/.egsnrcrc` | legacy-fallback | Read-only `EGS_HOME` fallback |
| `~/.bashrc` EGS blocks | legacy-fallback | Replaced by XDG one-liner |

## Touch points

| Component | Read / write | Migration |
|-----------|--------------|-----------|
| `HEN_HOUSE/scripts/configure` | writes user `.conf` to XDG | done |
| `HEN_HOUSE/scripts/configure_c++` | writes `egspp_*.conf` next to main conf | done |
| `HEN_HOUSE/scripts/finalize_egs_foruser` | writes profile `env`, prints one-liner | done |
| `HEN_HOUSE/scripts/egsnrc_config_paths` | XDG path helpers | new |
| `HEN_HOUSE/scripts/egsnrc` | profile CLI (`egsnrc use`, `egsnrc sync`) | new |
| `HEN_HOUSE/scripts/egsnrc_migrate_config` | legacy → XDG | new |
| `HEN_HOUSE/scripts/egsnrc_bashrc_additions` | sources from XDG if `EGS_CONFIG` unset | updated |
| `HEN_HOUSE/scripts/switch_config_bashrc` | legacy wrapper → `egsnrc use` | updated |
| `HEN_HOUSE/gui/egs_configure/egs_install_env.cpp` | GUI RC writer | pending |
| `HEN_HOUSE/scripts/compile_user_code` | uses `EGS_CONFIG` env | unchanged |
| `HEN_HOUSE/specs/all_common.spec` | `USER_SPEC_DIR` default for egspp includes | updated |
| 74 Makefiles | `$(USER_SPEC_DIR)egspp_<machine>.conf` only; `egspp_libs.spec` stays in `$(SPEC_DIR)` | updated |

## Primary UX

```bash
egsnrc use clrp-dev      # switch parallel install
egsnrc list              # list profiles
egsnrc config use debug  # switch machine config within profile
egsnrc sync              # refresh Makefiles from HEN_HOUSE → EGS_HOME
egsnrc sync user-codes egs_app  # full re-copy of one user code
```

Shell RC one-liner:

```bash
[ -f "${XDG_CONFIG_HOME:-$HOME/.config}/EGSnrc/EGSnrc.bash" ] && \
  . "${XDG_CONFIG_HOME:-$HOME/.config}/EGSnrc/EGSnrc.bash"
```
