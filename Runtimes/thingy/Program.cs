using ObjectIR.Core.Builder;
using ObjectIR.Core.IR;
using ObjectIR.Core.Serialization;
namespace ObjectIR.tools
{

    class Program
    {
        public static void Main(string[] args)
        {

            // Create a module
            var builder = new IRBuilder("HelloWorld");

            // Define a class with a Main method
#pragma warning disable CS8602 // Dereference of a possibly null reference.
            builder.Class("Program")
                .Namespace("HelloWorld")
                .Method("Main", TypeReference.Void)
                    .Static()
                        .Body()
                        .LdcR4(5.2f)
                        .Call(
                            new MethodReference(TypeReference.Float64, "System.Math.Sqrt", TypeReference.Float64, new List<TypeReference> { TypeReference.Float64 })
                            )
                        .Ret()
                    .EndBody()
                    .EndMethod()
                .EndClass();
#pragma warning restore CS8602 // Dereference of a possibly null reference.

            var module = builder.Build();
            Console.WriteLine(module.Serialize().DumpToJson());
        }
    }
}