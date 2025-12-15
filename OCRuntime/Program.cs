using ObjectIR;
using ObjectIR.Core.Builder;
using ObjectIR.Core.Composition;
using ObjectIR.Core.IR;
using ObjectIR.Core.Serialization;
using ObjectIR.Stdio;
using Math = ObjectIR.Stdio.Math;
namespace OCRuntime
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var builder = new IRBuilder("HelloWorld");

            // Define a class with a Main method
#pragma warning disable CS8602 // Dereference of a possibly null reference.
            builder.Class("Program")
                .Namespace("HelloWorld")
                .Method("Main", TypeReference.Void)
                    .Static()
                        .Body()
                        .Ldstr("meow")
                        .Call(Systems.WriteLine)
                        .Ret()
                    .EndBody()
                    .EndMethod()
                .EndClass();
#pragma warning restore CS8602 // Dereference of a possibly null reference.
            
            var program = builder.Build().DumpJson();
            Console.WriteLine(program);
            var Runtime = new IRRuntime(program);
            Runtime.Run();
        }
    }
}