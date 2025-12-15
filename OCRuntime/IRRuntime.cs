using MongoDB.Bson.IO;
using ObjectIR.Core.IR;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace OCRuntime
{

    public class IRRuntime
    {
        private Rootobject program;
        private Stack<CallFrame> callStack = new();
        public Action<CallFrame, Instruction>? OnStep;
        public delegate object? NativeMethod(object?[] args);
        private readonly Dictionary<string, NativeMethod> nativeMethods = new();

        public IRRuntime(string programJson)
        {
            program = JsonSerializer.Deserialize<Rootobject>(programJson);

            nativeMethods["System.Console.WriteLine(string)"] =
                args =>
                {
                    Console.WriteLine(args[0]);
                    return null;
                };
        }


        public void Run()
        {
            Console.WriteLine("Running Module: " + program.name);

            var programType = program.types.Single(t => t.name == "Program");
            var entry = programType.methods.Single(m => m.name == "Main" && m.isStatic);

            callStack.Push(new CallFrame(entry));
            Execute();
        }

        private void Execute()
        {
            while (callStack.Count > 0)
            {
                var frame = callStack.Peek();

                if (frame.IP >= frame.Method.instructions.Length)
                {
                    callStack.Pop();
                    continue;
                }

                var instr = frame.Method.instructions[frame.IP++];
                ExecuteInstruction(frame, instr);
            }
        }

        private void ExecuteInstruction(CallFrame frame, Instruction instr)
        {
            switch (instr.opCode)
            {
                case "ldc.i32":
                    frame.EvalStack.Push(int.Parse(instr.operand.value));
                    break;

                case "add":
                    {
                        var b = (int)frame.EvalStack.Pop();
                        var a = (int)frame.EvalStack.Pop();
                        frame.EvalStack.Push(a + b);
                        break;
                    }

                case "ret":
                    {
                        object? returnValue = null;

                        if (frame.EvalStack.Count > 0)
                            returnValue = frame.EvalStack.Pop();

                        callStack.Pop();

                        if (callStack.Count > 0 && returnValue != null)
                            callStack.Peek().EvalStack.Push(returnValue);

                        break;
                    }
                case "ldc":
                case "ldstr":
                    frame.EvalStack.Push(instr.operand.value);
                    break;

                case "call":
                    {
                        var target = instr.operand.method;

                        var signature =
                            $"{target.declaringType}.{target.name}({string.Join(",", target.parameterTypes)})";

                        // 🔹 Native method?
                        if (nativeMethods.TryGetValue(signature, out var native))
                        {
                            var args = new object?[target.parameterTypes.Length];

                            for (int i = args.Length - 1; i >= 0; i--)
                                args[i] = frame.EvalStack.Pop();

                            var result = native(args);

                            if (target.returnType != "void")
                                frame.EvalStack.Push(result);

                            break;
                        }

                        // 🔹 ObjectIR method
                        var declaringType = program.types
                            .Single(t => t.name == target.declaringType);

                        var method = declaringType.methods
                            .Single(m => m.name == target.name);

                        var newFrame = new CallFrame(method);

                        // move arguments
                        for (int i = target.parameterTypes.Length - 1; i >= 0; i--)
                            newFrame.EvalStack.Push(frame.EvalStack.Pop());

                        callStack.Push(newFrame);
                        break;
                    }
                default:
                    throw new NotSupportedException($"Unknown opcode {instr.opCode}");
            }
        }
        public class CallFrame
        {
            public Method Method { get; }
            public int IP { get; set; }
            public Stack<object?> EvalStack { get; } = new();

            public CallFrame(Method method)
            {
                Method = method;
                IP = 0;
            }
        }


        public class Rootobject
        {
            public string name { get; set; }
            public string version { get; set; }
            public Metadata metadata { get; set; }
            public Type[] types { get; set; }
            public object[] functions { get; set; }
        }

        public class Metadata
        {
        }

        public class Type
        {
            public string kind { get; set; }
            public string name { get; set; }
            public string _namespace { get; set; }
            public string access { get; set; }
            public bool isAbstract { get; set; }
            public bool isSealed { get; set; }
            public object[] interfaces { get; set; }
            public object[] baseInterfaces { get; set; }
            public object[] genericParameters { get; set; }
            public object[] fields { get; set; }
            public Method[] methods { get; set; }
            public object[] properties { get; set; }
        }

        public class Method
        {
            public string name { get; set; }
            public string returnType { get; set; }
            public string access { get; set; }
            public bool isStatic { get; set; }
            public bool isVirtual { get; set; }
            public bool isOverride { get; set; }
            public bool isAbstract { get; set; }
            public bool isConstructor { get; set; }
            public object[] parameters { get; set; }
            public object[] localVariables { get; set; }
            public int instructionCount { get; set; }
            public Instruction[] instructions { get; set; }
        }

        public class Instruction
        {
            public string opCode { get; set; }
            public Operand operand { get; set; }
        }

        public class Operand
        {
            public string value { get; set; }
            public string type { get; set; }
            public Method1 method { get; set; }
        }

        public class Method1
        {
            public string declaringType { get; set; }
            public string name { get; set; }
            public string returnType { get; set; }
            public string[] parameterTypes { get; set; }
        }

    }
}