# `iptable_evil`

`iptable_evil` is a very specific backdoor for `iptables` that allows all packets with the evil bit set, no matter the firewall rules.

The initial implementation is in `iptable_evil.c`, which adds a table to `iptables` and requires modifying a kernel header to insert a spot for it.
The second implementation is a modified version of the `ip_tables` core module and its dependents to allow all Evil packets.

I have tested it on Linux kernel version 5.8.0-48, but this should be appliciable to pretty much any kernel version with a full implementation of iptables.

## Explanation of the Evil Bit

[RFC3514](https://tools.ietf.org/html/rfc3514), published April 1st, 2003, defines the previously-unused high-order bit of the IP fragment offset field as a security flag.
To RFC-compliant systems, a `1` in that bit position indicates evil entent and will cause the packet to be blocked.

By default, this bit is turned off, but can be turned on in your software if you're assembling the entirety of your IP packet (as some hacking tools do), or in the Linux kernel using [this patch](https://blog.benjojo.co.uk/asset/bxwi3gFqKd) (mirrored in this repository [here](https://github.com/FlamingSpork/iptable_evil/blob/main/replace-existing/net/ipv4/ip_output.c)).

## How does the backdoor work?

When a packet is received by the Linux kernel, it is processed by `iptables` and either sent to userspace, rejected, or modified based on the rules configured.

In particular, each `iptables` table uses the function `ipt_do_table` in `ip_tables.c` to decide whether to accept a given packet.
I have modified that to automatically accept any packet with the evil bit set and skip al further processing.

I also attempted to add another table (`iptable_evil.c`) that would accept all evil packets and hand others off to the standard tables for processing, but I never figured out how to pass the packets to the next table and decided that the `ipt_do_table` backdoor was enough as a proof of concept.

## Why did you do this?
I needed to do and write up a decently large project in computing security for one of my classes, and this seemed like a cool idea.
This is probably more work than he was expecting for this but ¯\\\_(ツ)_/¯.


# Build

## In-Tree Build

The `evil` table requires modification of kernel headers, so installing it requires running with a kernel produced through the full tree build.

* Copy the contents of `replace-existing` to your kernel source tree, overwriting existing files.
* Copy `iptable_evil.c` to `linux-X.Y.Z/net/ipv4/netfilter`
* (optional) copy `ip_tables.c` to `linux-X.Y.Z/net/ipv4/netfilter`
* Compile the kernel according to your distro's process (should produce a package)
* Install the package file
* Reboot into your new kernel
* `iptables -t filter -L`
* `iptables -t evil -L` (this will have confused output, but it will load the module)

## Out-of-Tree Build

This is significantly easier and faster, but does not support the `evil` table and marks the kernel as "tainted".
It *should* be possible to copy the `ko` files produced by this to another computer with the exact same kernel version,
but I haven't tested it.

* Run `make`
* `rmmod iptable_*`
* `rmmod ip_tables`
* `insmod ip_tables.ko`
* `insmod iptable_filter.ko`

# Testing/Demo
**TODO: explain setting evil bit and so forth**
**TODO: include screenshots**

## Kernel Version
* 5.8.0-48-generic (Ubuntu 20.04)

# Further Information and Resources

* RFC 3514 inventing the evil bit: https://tools.ietf.org/html/rfc3514
* Ben Cox's introduction to the evil bit: https://blog.benjojo.co.uk/post/evil-bit-RFC3514-real-world-usage
* Ben Cox's `iptables_uwu` (mostly just for giving the names of things to research): https://github.com/benjojo/iptables-uwu
* A somewhat outdated but *very* detailed explanation of how `iptables` works and how to add targets and modules to it: https://inai.de/documents/Netfilter_Modules.pdf
* https://upload.wikimedia.org/wikipedia/commons/3/37/Netfilter-packet-flow.svg
* Bootlin's Elixir search is significantly easier to use to find identifiers in the kernel than grep: https://elixir.bootlin.com/linux/v5.8/source/net/ipv4/netfilter/ip_tables.c#L225
* Ubuntu's docs explain how to build the kernel: https://wiki.ubuntu.com/Kernel/BuildYourOwnKernel
