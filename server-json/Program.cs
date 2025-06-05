using System;
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

class Program
{
    static async Task SendRequestAsync(object data)
    {
        using var client = new TcpClient();
        await client.ConnectAsync("127.0.0.1", 8888);

        var stream = client.GetStream();
        string json = JsonSerializer.Serialize(data);
        byte[] bytes = Encoding.UTF8.GetBytes(json);
        await stream.WriteAsync(bytes, 0, bytes.Length);

        byte[] buffer = new byte[4096];
        int byteCount = await stream.ReadAsync(buffer, 0, buffer.Length);
        string response = Encoding.UTF8.GetString(buffer, 0, byteCount);

        Console.WriteLine("Response:");
        Console.WriteLine(response);
    }

    static async Task Main()
    {
        await SendRequestAsync(new
        {
            method = "GET",
            route = "/table/list",
            payload = new { DatabaseId = 1 }
        });

        await SendRequestAsync(new
        {
            method = "POST",
            route = "/table/123/rows",
            payload = new { Filter = "age > 30" }
        });
    }
}
