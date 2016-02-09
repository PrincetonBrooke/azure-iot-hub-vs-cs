#include <ppltasks.h>
#include <pplawait.h>
#include <stdio.h>

using namespace Microsoft::Azure::Devices::Client;
using namespace Platform;
using namespace concurrency;

//
// String containing Hostname, Device Id & Device Key in the format:
// "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"
//
// Note: this connection string is specific to the device "$deviceId$". To configure other devices,
// see information on iothub-explorer at http://aka.ms/iothubgetstartedVSCS
//
static const wchar_t* connection_string = L"HostName=$iotHubUri$;DeviceId=$deviceId$;SharedAccessKey=$deviceKey$";

//
// To monitor messages sent to device "$deviceId$" use iothub-explorer as follows:
//    iothub-explorer HostName=$iotHubUri$;SharedAccessKeyName=service;SharedAccessKey=$servicePrimaryKey$ monitor-events "$deviceId$"
//

task<void> send_device_to_cloud_message()
{
    auto deviceClient = DeviceClient::CreateFromConnectionString(ref new Platform::String(connection_string), TransportType::Http1);

    std::string message = "Hello from C++!";

    auto pbuffer = ref new Platform::Array<byte>((unsigned char*)message.data(), message.length());
    auto eventMessage = ref new Message(pbuffer);
    return create_task(deviceClient->SendEventAsync(eventMessage)).then([] {
        OutputDebugString(L"message sent successfully\n");
    });
}

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED
task<std::string> receive_cloud_to_device_message()
{
    auto deviceClient = DeviceClient::CreateFromConnectionString(ref new Platform::String(connection_string), TransportType::Http1);

    while (true)
    {
        auto receivedMessage = await deviceClient->ReceiveAsync();

        if (receivedMessage != nullptr)
        {
            auto bytes = receivedMessage->GetBytes();
            auto messageData = bytes->Data;
            auto len = bytes->Length;

            std::string str((char*)messageData, len);

            await deviceClient->CompleteAsync(receivedMessage);

            return str;
        }

        //  Note: In this sample, the polling interval is set to 
        //  10 seconds to enable you to see messages as they are sent.
        //  To enable an IoT solution to scale, you should extend this 
        //  interval. For example, to scale to 1 million devices, set 
        //  the polling interval to 25 minutes.
        //  For further information, see
        //  https://azure.microsoft.com/documentation/articles/iot-hub-devguide/#messaging

        await create_task([] { std::this_thread::sleep_for(std::chrono::seconds(10)); });
    }
}
#endif
