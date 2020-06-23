Troubleshooting SDK Issues
--------------------------

1.  **Console shows “Couldn’t resolve the host name”.**

    It is because the time out setting in Linux demo doesn’t fit the network
    time out pattern of your own Raspberry Pi.

2.  **If console always shows “ [DEBUG] ffsRaspbianPerformDirectedScan:114:
    Directed scan did not find the given SSID” until the end of the demo.**

    This means the provisionee can’t find a SSID provided by the provisioner. In
    this case, you can check if the provisioner is on a 2.4 or 4GHz wifi, or if
    the provisioner is registered under the same customerId where you associate
    the test certification.

3.  If the log showing SSID is found, and starting showing log connecting to dps
    cloud, but fail at the last step. It means the customer with test
    certificates associated might not have wifi under the locker, or the
    certificate is not placed correctly under Linux demo folder.

4.  For Raspberry PI, if run at the end, and see “[ERROR]
    ffsHttpExecutePreallocated: 200: CURL operation curl_easy_perform(session) ”
    or other CURL related errors. It’s related to the Raspberry PI network
    intermittent settings, will not appear on real embedded device.
