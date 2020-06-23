WSS C SDK:
==========

The WSS SDK currently is a C Based SDK so it can be ported to different
platforms and chipsets. Firmware developers will need to implement a
compatibility layer specific to the platform that you will port the SDK on.
Frustration Free Setup team has included an implementation of a compatibility
layer that works with Embedded Linux, MAC, and Raspberry Pi so developers can
immediately download the sdk and run a demo program within the SDK on any of those platforms. The below
tasks are leveraging that compatibility layer and provides a way to accelerate
running WSS sdk. The following are the tasks in this category:



1.  Download and Run WSS C SDK

2.  Build WSS SDK with AWS IoT for embedded Linux.

Prerequisites
-------------

The following is the list of pre-requisites that you need to make sure are met
in order to be able to perform any tasks with WSS CSDK. The prerequisites are:

1.  A Valid Provisioner. The following are the list of valid provisioners that
    should be in proximity of the device running the demo:

    1.  Echo dot gen2(donut). Only if connected to 2.4 GHz network.

    2.  Echo dot gen3(crumpet). Only if connected to 2.4 GHz network.

    3.  Eero mesh, TP-Link Archer A7, or ASUS rt-ax88u Routers that with hidden SSID "simple_setup".

    4.  Your own router with creating a hidden 2.4GHz SSID called
        “simple_setup”.

2.  Valid Keys for the device. Static Keys can be requested through the FFS
    developer console and it will be automatically associated with the developer's 
    Amazon account that requested the static keys. The default location that the sdk
    libraries load the device keys from is
    \$ffs/ffs-provisionee-sdk-master/ffs_linux/libffs/data/device_certificate 

3.  Device keys belong to a customer that has his Wi-Fi password stored in
    Amazon Wi-Fi Locker. This could be verified by setting up any Amazon (echo,
    echo dot, firetv, firecube, etc..) with the customer account that is
    associated with the keys being used by the SDK.


[Next\>\>](Task-SDK001.md)
