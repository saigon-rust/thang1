using System;
using System.Net.Sockets;
using System.Security.Cryptography;
using System.Text;
using System.Threading;

class Program
{
    const string ServerIP = "127.0.0.1";
    const int ServerPort = 12345;

    static RSA? rsa = null;    // Server public key
    static Aes? aes = null;    // AES key/IV hiện tại
    static TcpClient? client = null;
    static NetworkStream? stream = null;

    static DateTime lastKeySent = DateTime.MinValue;

    static void Main()
    {
        try
        {
            client = new TcpClient(ServerIP, ServerPort);
            stream = client.GetStream();

            ReceiveServerPublicKey();

            GenerateAndSendAESKey();

            // Thread gửi key mới mỗi 10s
            new Thread(() =>
            {
                while (true)
                {
                    if ((DateTime.Now - lastKeySent).TotalSeconds >= 10)
                    {
                        GenerateAndSendAESKey();
                    }
                    Thread.Sleep(500);
                }
            }) { IsBackground = true }.Start();

            while (true)
            {
                Console.Write("Enter message: ");
                string? input = Console.ReadLine();
                if (string.IsNullOrEmpty(input)) break;

                SendEncryptedMessage(input);
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
            rsa?.Dispose();
            aes?.Dispose();
        }
    }

    // Nhận PEM public key server gửi (ASCII)
    static void ReceiveServerPublicKey()
    {
        if (stream == null) throw new Exception("Stream not ready.");

        byte[] buffer = new byte[4096];
        int len = stream.Read(buffer, 0, buffer.Length);
        string pem = Encoding.ASCII.GetString(buffer, 0, len);

        rsa = RSA.Create();
        rsa.ImportFromPem(pem.ToCharArray());

        Console.WriteLine("Received server public key.");
    }

    // Sinh AES key+IV mới và gửi cho server (RSA encrypt)
    static void GenerateAndSendAESKey()
    {
        if (rsa == null || stream == null) throw new Exception("RSA or stream not ready.");

        aes?.Dispose();
        aes = Aes.Create();
        aes.KeySize = 256;
        aes.Mode = CipherMode.CBC;
        aes.Padding = PaddingMode.PKCS7;
        aes.GenerateKey();
        aes.GenerateIV();

        // Chuẩn bị buffer key+iv 48 bytes
        byte[] keyiv = new byte[aes.Key.Length + aes.IV.Length];
        Buffer.BlockCopy(aes.Key, 0, keyiv, 0, aes.Key.Length);
        Buffer.BlockCopy(aes.IV, 0, keyiv, aes.Key.Length, aes.IV.Length);

        // Mã hóa RSA OAEP
        byte[] encrypted = rsa.Encrypt(keyiv, RSAEncryptionPadding.OaepSHA1);

        SendPacket(encrypted);

        lastKeySent = DateTime.Now;
        Console.WriteLine("Sent new AES key and IV.");
    }

    // Mã hóa AES-CBC với key+IV hiện tại và gửi
    static void SendEncryptedMessage(string plaintext)
    {
        if (aes == null || stream == null) throw new Exception("AES or stream not ready.");

        byte[] plainBytes = Encoding.UTF8.GetBytes(plaintext);

        // Tạo bộ encryptor mới (đảm bảo IV đúng, không bị thay đổi)
        using ICryptoTransform encryptor = aes.CreateEncryptor();

        byte[] encrypted = encryptor.TransformFinalBlock(plainBytes, 0, plainBytes.Length);

        SendPacket(encrypted);

        Console.WriteLine("Sent encrypted message.");
    }

    // Gửi packet với 4 byte độ dài (big endian) + dữ liệu
    static void SendPacket(byte[] data)
    {
        if (stream == null) throw new Exception("Stream not ready.");

        byte[] lenBytes = BitConverter.GetBytes(data.Length);
        if (BitConverter.IsLittleEndian) Array.Reverse(lenBytes);

        stream.Write(lenBytes, 0, 4);
        stream.Write(data, 0, data.Length);
    }
}
