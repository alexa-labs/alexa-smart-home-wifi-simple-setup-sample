### Implement CDAE Directive

Make sure you have completed the steps associated with Implementing SDAE Event
and the implementation of the CDAE will be included in the file:
lambda/api/endpoint_cloud/api_handler_directive.py. CDAE is similar to smart
home directives where namespace will be “Alexa.SimpleSetup” and the name will be
“AssociateCustomerDevice” and Alexa will send this directive to the skill in
case while processing Skill Device Association Event alexa backedn found that
the customer that provisioned this device has the skill enabled and ready to be
used.

This is the final stage of the guide where you can run the end to end test
starting from running the sdk demo with the appropriate keys and wait till Alexa
announces that there is a new device discovered.

To run the end to end demo, please follow the following:

1.  Navigate to `wss-workshop-sample-device/ffs-provisionee-sdk-master/ffs_linux/build`

2.  Replace the valid keys(`certificate.pem and private_key.pem`) that belong to
    the test customer into the directory`data/device_certificate/`

3.  Make sure that the skill you are testing with has been enabled for the same
    customer.

4.  Navigate back to the build folder and run the workshop executable with
    passing the provisioning url. The porvisioning url should be API url you
    configured in [Implement SDAE Event](Task-API001.md) Command should look
    like the below:

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ./FrustrationFreeSetupLinuxWorkshop-mac -u -u https://{API-Id}.execute-api.us-east-1.amazonaws.com/prod/provision 
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

5.  Expected Result: A Notification from Alexa that says "Your First Bulb is
    connected"
