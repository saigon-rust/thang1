using System;
using System.IO;
using System.Net.Sockets;
using System.Security.Cryptography;
using System.Text;
using System.Threading;

class RSAClient
{
    const int Port = 12345;
    const string ServerAddress = "127.0.0.1";
    static RSA rsa;
    static Aes aes;
    static TcpClient client;
    static NetworkStream stream;
    static Timer aesRotationTimer;
    static readonly object aesLock = new();

    static void Main()
    {
        Console.WriteLine("Connecting to server...");
        client = new TcpClient(ServerAddress, Port);
        stream = client.GetStream();

        ReceiveAndLoadServerPublicKey();

        GenerateNewAESKey();
        SendEncryptedAESKey();

        // Tự động xoay AES key mỗi 10 giây
        aesRotationTimer = new Timer(RotateAESKey, null, TimeSpan.FromSeconds(10), TimeSpan.FromSeconds(10));

        Console.WriteLine("Type message to send (type 'exit' to quit):");

        while (true)
        {
            Console.Write("> ");
            string message = Console.ReadLine();

            if (string.IsNullOrWhiteSpace(message)) continue;
            if (message.Equals("exit", StringComparison.OrdinalIgnoreCase)) break;

            SendEncryptedMessage(message);
        }

        aesRotationTimer.Dispose();
        stream.Close();
        client.Close();
    }

    static void ReceiveAndLoadServerPublicKey()
    {
        byte[] buffer = new byte[4096];
        int len = stream.Read(buffer, 0, buffer.Length);
        string pubPem = Encoding.ASCII.GetString(buffer, 0, len);

        rsa = RSA.Create();
        rsa.ImportFromPem(pubPem.ToCharArray());

        Console.WriteLine("Loaded server public key.");
    }

    static void GenerateNewAESKey()
    {
        lock (aesLock)
        {
            aes?.Dispose();
            aes = Aes.Create();
            aes.KeySize = 256;
            aes.GenerateKey();
            aes.GenerateIV();
        }

        Console.WriteLine("Generated new AES key/IV.");
    }

    static void RotateAESKey(object state)
    {
        GenerateNewAESKey();
        SendEncryptedAESKey();
        Console.WriteLine("Rotated AES key (every 10 seconds).");
    }

    static void SendEncryptedAESKey()
    {
        lock (aesLock)
        {
            byte[] payload = new byte[aes.Key.Length + aes.IV.Length];
            Buffer.BlockCopy(aes.Key, 0, payload, 0, aes.Key.Length);
            Buffer.BlockCopy(aes.IV, 0, payload, aes.Key.Length, aes.IV.Length);

            byte[] encryptedKey = rsa.Encrypt(payload, RSAEncryptionPadding.OaepSHA1);
            SendPacket(encryptedKey);
        }

        Console.WriteLine("Sent AES key to server.");
    }

    static void SendEncryptedMessage(string plaintext)
    {
        byte[] plainBytes = Encoding.UTF8.GetBytes(plaintext);
        byte[] cipherBytes;

        lock (aesLock)
        {
            using var encryptor = aes.CreateEncryptor();
            cipherBytes = encryptor.TransformFinalBlock(plainBytes, 0, plainBytes.Length);
        }

        SendPacket(cipherBytes);
        Console.WriteLine("Sent encrypted message.");
    }

    static void SendPacket(byte[] data)
    {
        byte[] lenBytes = BitConverter.GetBytes(data.Length);
        if (BitConverter.IsLittleEndian)
            Array.Reverse(lenBytes); // big-endian

        stream.Write(lenBytes, 0, 4);
        stream.Write(data, 0, data.Length);
    }
}
