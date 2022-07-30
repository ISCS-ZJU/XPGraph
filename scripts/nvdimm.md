# NVDIMM

## Provision Memory Mode

First delete the current namespaces and 'goal' which determines the region configuration at boot time. Then install the new DCPMM and create a new goal. Finally, you can then create the namespaces.

```bash
# Check for namespaces
ndctl list -iN
# Delete all valid namespaces on all regions
ndctl destroy-namespace -f all
# Delete the region goal
ipmctl delete -goal
# reboot the host
sudo systemctl reboot
```

Configures all the pmem module capacity in Memory Mode.

```bash
ipmctl create -goal MemoryMode=100
```

## Provision App Direct

First delete the current namespaces and 'goal' which determines the region configuration at boot time. Then configures all the pmem module capacity in AppDirect mode with all modules in an interleaved set.

```bash
# Check for namespaces
ndctl list -iN
# Delete all valid namespaces on all regions
ndctl destroy-namespace -f all
# Delete the region goal
ipmctl delete -goal
# reboot the host
systemctl reboot
```

Configures all the pmem module capacity in Memory Mode.
```bash
ipmctl create -goal PersistentMemoryType=AppDirect
```

Then reboot the host and create new namespace as `fsdax` mode until no space left on device.

```bash
# run N times until no space left on device
ndctl create-namespace --mode=fsdax 
```

Finally, to get the DAX functionality, mount the file system with the dax mount option. File system creation requires no special arguments.

```bash
mkfs.ext4 -b 4096 -E stride=512 -F ${device_path_0}
mount -o dax ${device_path_0} ${pmem0_path}
mkfs.ext4 -b 4096 -E stride=512 -F ${device_path_1}
mount -o dax ${device_path_1} ${pmem1_path}
...
```

