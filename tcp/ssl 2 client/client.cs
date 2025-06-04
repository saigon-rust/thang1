// RSAClient.cs
using System;
using System.Net.Sockets;
using System.Security.Cryptography;
using System.Text;

class RSAClient
{
    static void Main()
    {
        TcpClient client = new TcpClient("127.0.0.1", 12345);
        NetworkStream stream = client.GetStream();

        // Read public key
        byte[] pubKeyBytes = new byte[4096];
        int len = stream.Read(pubKeyBytes, 0, pubKeyBytes.Length);
        string pubKeyPem = Encoding.ASCII.GetString(pubKeyBytes, 0, len);

        // Convert PEM to RSA key
        RSA rsa = RSA.Create();
        rsa.ImportFromPem(pubKeyPem.ToCharArray());

        string message = "SensitiveData123!";
        byte[] encrypted = rsa.Encrypt(Encoding.UTF8.GetBytes(message), RSAEncryptionPadding.Pkcs1);

        // Send encrypted message
        stream.Write(encrypted, 0, encrypted.Length);
        Console.WriteLine("Encrypted message sent.");

        stream.Close();
        client.Close();
    }
}
