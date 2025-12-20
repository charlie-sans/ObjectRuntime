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
            // Hardcoded Text IR HelloWorld
            var textIr = @"module HelloWorld
class Program {
    static method Main() -> void {
        ldstr ""Hello from Text IR!""
        call System.Console.WriteLine(string) -> void
        ret
    }
}
";
            Console.WriteLine("--- Text IR Input ---\n" + textIr);
            var runtime = new IRRuntime(textIr);
            runtime.Run();
        }
    }
}