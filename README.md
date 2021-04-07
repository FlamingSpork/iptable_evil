# `iptable_evil`

`iptable_evil` is a very specific backdoor for `iptables` that allows all packets with the [Evil Bit](https://tools.ietf.org/html/rfc3514) set, no matter the firewall rules.

The initial implementation is in `iptable_evil.c`, which adds a table to `iptables` and requires modifying a kernel header to insert a spot for it.
The second implementation is a modified version of the `ip_tables` core module and its dependents to allow all Evil packets.

**TODO: tested on which version?**

## Explanation of the Evil Bit
**TODO**

## In-Tree Build

The `evil` table requires modification of kernel headers, so installing it requires running with a kernel produced through the full tree build.

* Copy the contents of `replace-existing` to your kernel source tree, overwriting existing files.
* Copy `iptable_evil.c` to `linux-X.Y.Z/net/ipv4/netfilter`
* (optional) copy `ip_tables.c` to `linux-X.Y.Z/net/ipv4/netfilter`
* Compile the kernel according to your distro's process (should produce rpm or deb)
* Install the package file
* Reboot into your new kernel
* `iptables -t filter -L`
* (optional) `iptables -t evil -L`

## Out-of-Tree Build

This is significantly easier and faster, but does not support the `evil` table and marks the kernel as "tainted".
It *should* be possible to copy the `ko` files produced by this to another computer with the exact same kernel version,
but I haven't tested it.

* Run `make`
* `rmmod iptable_*`
* `rmmod ip_tables`
* `insmod ip_tables.ko`
* `insmod iptable_filter.ko`

## Further Information and Resources

* RFC 3514: https://tools.ietf.org/html/rfc3514
* Ben Cox's introduction to the evil bit: https://blog.benjojo.co.uk/post/evil-bit-RFC3514-real-world-usage
* Ben Cox's iptables_uwu: https://github.com/benjojo/iptables-uwu
* A (somewhat outdated) explanation of how `iptables` works: https://inai.de/documents/Netfilter_Modules.pdf
* https://upload.wikimedia.org/wikipedia/commons/3/37/Netfilter-packet-flow.svg
* Bootlin's Elixir search is significantly easier to use to find identifiers in the kernel than grep: https://elixir.bootlin.com/linux/v4.7/source/net/ipv4/netfilter/ip_tables.c#L618
* Ubuntu's docs explain how to build the kernel: https://wiki.ubuntu.com/Kernel/BuildYourOwnKernel
