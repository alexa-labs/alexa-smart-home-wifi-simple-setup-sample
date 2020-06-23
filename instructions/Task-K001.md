Task-K001 – Request a Static keys and Certificates
--------------------------------------------------

Make sure that you are logged in to the [Frustration-Free Setup
Console](https://developer.amazon.com/frustration-free-setup/console) using a
valid developer account. The keys that will be generated will be a set of 5
static keys that are associated with that developer account. So, in order to
make use of the keys, you will need to test them in an environment where an Echo
device has been setup using that developer account.

1.  Open the Wi-Fi Simple Setup tab in the Frustration-Free Setup console, and
    click on 3. Request Static Keys And Certificates.

    ![](media/image1.png)

2.  Populate the full details about the product that you’re planning to use for
    Wi-Fi Simple Setup.

    ![](media/image2.png)

    The device data collected are mainly used for onboarding to Amazon Device
    Management Service. The following table shows what every fields will be used
    for by Amazon:

| **Device Meta Data** | **Description**                                                                                                                                    |
|----------------------|----------------------------------------------------------------------------------------------------------------------------------------------------|
| Contact Name         | Name of the Developer that will be using the keys                                                                                                  |
| Contact Email        | Email address to receive the static keys generated.                                                                                                |
| Product Manufacturer | Name of the manufacturer. Used for reference.                                                                                                      |
| Manufacturer Website | Website of the manufacturer. Used for reference only.                                                                                              |
| Product Name         | Name of the product. This will be used for identifying the device. It will drive a device type and Product Id in Amazon Device Management Service. |
| Product Description  | Description of product. Used for reference only.                                                                                                   |
| Product SKU          | SKU of the product. Will be used for reference in Amazon and can be changed later.                                                                    |
| Expected Launch Date | A planned date for launch. Used for reference only.                                                                                                |

3.  You should receive the following email from Frusteration-Free setup team
    within 5-10 business days with a link to the keys to download:

 

>   *Hello Your Name,                *

>   *This email is to deliver the Wi-Fi Frustration-Free Setup static keys and
>   certificates you requested for testing your provisionee device with SKU:
>   ashbdsa-123.                *

>   *Before downloading, please note a few disclaimers regarding this process:*

>   *Please note that the provisioner device you use to test your Wi-Fi Simple
>   Setup provisionee must currently either be a 2nd-generation Echo device
>   (https://www.amazon.com/dp/B06XCM9LJ4/)https://www.amazon.com/dp/B06XCM9LJ4/ or
>   an Eero router. Additionally, this provisioner must be associated to the
>   same Amazon account that you used to log into the Amazon Developer Console
>   when you requested the static keys and certificates.You can check this by
>   logging into the Alexa app using your Amazon account credentials. If you see
>   your provisioner device listed, then you should be good to go. If you would
>   like to change the account that the test keys and certificates are
>   associated with, please contact us and we will update your information and
>   send you a new batch of static keys to test with.*

>   *Download this zip
>   filehttps://frustration-free-setup-static-keys-download.s3.amazonaws.com/338b8fea-c216-440a-9b11-464243b02650/test_certificates.zip?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Date=20200217T235451Z&X-Amz-SignedHeaders=host&X-Amz-Expires=604799&X-Amz-Credential=AKIAUJBVWZ3DEBM2CFVG%2F20200217%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Signature=091b0102cebac4f731fce244059f0f2d1ebcb98dc62a7c3e72ba8faa883f0951,
>   which contains the static keys and certificates you can use to test your
>   Frustration-Free Setup provisionee device. Be aware that the link will
>   expire within 7 days.*

>   *Please contact us if you need anything or have any additional questions!*

>   *Frustration-Free Setup team*

 

1.  The following are the contents of the zip file that you will download from
    the link provided in the email:

![](media/image3.png)

 
Make sure unzip and save the folder in a secured location in your computer. 

[Next>>](wss-c-sdk.md)
