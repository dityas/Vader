import subprocess
import pathlib


def get_file_data(file_path):
    try:
        data = ""
        with open(file_path, 'r') as f:
            data = f.read()

        return data

    except Exception as e:
        print(f"[-] Error {e} while reading file {file_path}")
        return ""


def check_DMI_info():

    identifiers = ["KVM",
                   "OpenStack",
                   "KubeVirt",
                   "Amazon EC2",
                   "QEMU",
                   "VMware",
                   "VMW",
                   "innotek GmbH",
                   "VirtualBox",
                   "Xen",
                   "Bochs",
                   "Parallels",
                   "BHYVE",
                   "Hyper-V",
                   "Apple Virtualization"]

    DMI_FILES = ["/sys/class/dmi/id/product_name",
                 "/sys/class/dmi/id/sys_vendor",
                 "/sys/class/dmi/id/board_vendor",
                 "/sys/class/dmi/id/bios_vendor"]

    found = False
    for dmi_file in DMI_FILES:
        data = get_file_data(dmi_file)

        for identifier in identifiers:
            if identifier in data:
                print(f"[X] Found {identifier} in {dmi_file}")
                found = True

    if found:
        return True
    
    else:
        print("[ ] Nothing suspicious found in dmi files")
        return False


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
    return False


def check_known_dev_files():
    known_dirs = set(["vboxuser", "vboxguest"])

    for dev in pathlib.Path("/dev").iterdir():
        if dev.parts[-1] in known_dirs:
            print(f"[X] Found device file {dev} associated with VBox")
            return True

    print("[ ] No known /dev files found")
    return False


def check_usb_device():

    lsusb = subprocess.run("lsusb",
                           stdout=subprocess.PIPE).stdout.decode("utf8")

    for line in lsusb.splitlines():
        if "VirtualBox" in line:
            print(f"[X] Found USB device log {line}")
            return True

    print("[ ] No virtual USB devices detected")
    return False


if __name__ == "__main__":
    check_known_MAC_ifaces()
    check_known_dev_files()
    check_usb_device()
    check_DMI_info()
