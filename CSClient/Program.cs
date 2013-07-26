using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LCouchbase;

namespace CSClient
{
    class Program
    {
        static void Main(string[] args)
        {
            Couchbase client = new Couchbase("10.0.0.99", "default");
            byte[] key = Encoding.UTF8.GetBytes("Hello!");


            client.Connect();
            StoreCommand scmd = new StoreCommand(key);
            scmd.Value = Encoding.UTF8.GetBytes("blah blah");
            scmd.Mode = StoreMode.Set;
            var res = client.Store(scmd);
            Console.WriteLine("Got Cas: " + res.Cas);

            GetCommand gcmd = new GetCommand(key);
            res = client.Get(gcmd);
            Console.WriteLine("Got response (value)" +Encoding.UTF8.GetString(res.Value));
            Console.WriteLine("Cas is " + res.Cas);

        }
    }
}
