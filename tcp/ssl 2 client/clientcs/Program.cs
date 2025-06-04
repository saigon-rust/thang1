using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

class ChatClient
{
    static TcpClient client;
    static NetworkStream stream;

    static async Task ReceiveMessages()
    {
        byte[] buffer = new byte[1024];
        try
        {
            while (true)
            {
                int bytesRead = await stream.ReadAsync(buffer, 0, buffer.Length);
                if (bytesRead == 0)
                {
                    Console.WriteLine("\nDisconnected from server.");
                    Environment.Exit(0);
                }
                string msg = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                Console.WriteLine($"\nMessage from other client: {msg}\n> ");
            }
        }
        catch
        {
            Console.WriteLine("\nConnection lost.");
            Environment.Exit(0);
        }
    }

    static async Task Main()
    {
        client = new TcpClient();
        try
        {
            await client.ConnectAsync("127.0.0.1", 12345);
            stream = client.GetStream();

            Console.WriteLine("Connected to server. You can start chatting!");

            // Task nhận message
            _ = ReceiveMessages();

            // Đọc input từ bàn phím, gửi server
            while (true)
            {
                Console.Write("> ");
                string message = Console.ReadLine();
                if (string.IsNullOrEmpty(message)) continue;

                byte[] data = Encoding.UTF8.GetBytes(message);
                await stream.WriteAsync(data, 0, data.Length);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("Error: " + ex.Message);
        }
        finally
        {
            stream?.Close();
            client?.Close();
        }
    }
}
