

virt-install --name arch --ram 2048 \
  --disk path=/home/ashinde/UGA/SSCF/Vader/test_vm/arch_linux \
  --vcpus 2 \
  --os-type linux \
  --network bridge=virbr0 \
  --graphics none \
  --console pty,target_type=serial \
  --cdrom /home/ashinde/UGA/SSCF/Vader/test_vm/arch_install.iso
