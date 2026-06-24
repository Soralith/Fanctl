# fanctl

After years of suffering to not be able to control my laptop fan speed on Linux, I finally found out the Clevo Embedded Controller exposes CPU and GPU fan control separately via I/O ports. So I wrote this.

`fanctl` talks directly to the EC and tells it exactly what speed to run each fan at. No daemons, no GUI, no bullshit.

## What it does

- Set CPU fan speed to any percentage
- Set GPU fan speed to any percentage
- Reset either fan to automatic (let the EC decide again)
- Read current temperatures and fan RPM

## Usage

```
sudo fanctl --cpu 50 --gpu 70      # set CPU to 50%, GPU to 70%
sudo fanctl --gpu auto             # set GPU back to automatic
sudo fanctl --cpu auto --gpu auto  # both back to automatic
fanctl --status                    # show temps and fan info
fanctl --version                   # show version
```

No arguments are required. You can mix `--cpu`, `--gpu`, and `--status` in any order:

```
sudo fanctl --cpu 80
sudo fanctl --gpu 70 --status
sudo fanctl --cpu auto --gpu 60
```

## Install

```
git clone https://github.com/Soralith/Fanctl
cd Fanctl
sudo make install
```

This copies `fanctl` to `/usr/local/bin` and sets the setuid root bit so you don't need `sudo` every time.

## Compatibility

This works on Clevo-based laptops (TUXEDO, Schenker, Sager, Eluktronics, System76, and other Clevo rebrands) that use the standard Clevo EC protocol (ports `0x66`/`0x62`, command `0x99`). The specific EC register layout (temps at `0x07`/`0xCD`, fan register `0xCE`) is common across many Clevo models but not guaranteed.

If `fanctl --status` returns reasonable-looking temperatures and fan speeds, the rest will work too.

It is **not** compatible with non-Clevo laptops, even if they have the same GPU. The fan is wired through the EC, not the GPU itself.

## How it works

```
outb(0x99, 0x66)        # EC command: set fan
outb(0x01, 0x62)        # 0x01 = CPU fan, 0x02 = GPU fan
outb(duty_value, 0x62)  # speed from 0 to 255
```

The `auto` command sends `0xff` as the index, which tells the EC to resume its built-in curve for that fan.

## License

Do whatever you want with it.
