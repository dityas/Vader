import subprocess


def _get_interface_addrs(raw_strings):

    addrs = filter(lambda x: "link/" in x, raw_strings)
    addrs = map(lambda x: x.split()[1], addrs)
    return list(addrs)


def check_known_MAC_ifaces():
    output = subprocess.run(["ip", "link"],
                            stdout=subprocess.PIPE).stdout
    output = output.decode('utf8').splitlines()
    addrs = _get_interface_addrs(output)

    for addr in addrs:
        if "08:00:27" in addr:
            print(f"[X] Found iface {addr} commonly used with VBox")
            return True

    print("[ ] No known VM ifaces found")


if __name__ == "__main__":
    check_known_MAC_ifaces()
