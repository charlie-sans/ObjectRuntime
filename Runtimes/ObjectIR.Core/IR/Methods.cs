using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ObjectIR.Core.Builder;
using ObjectIR.Core.IR;

namespace ObjectIR.Stdio
{
    public class Systems
    {

        public static MethodReference WriteLine = new MethodReference(TypeReference.String, "System.Console.WriteLine", TypeReference.String, [TypeReference.String]);
    }

    /// <summary>
    /// Bindings to ObjectIR.System.Math.* functions
    /// </summary>
    public class Math
    {
        /// <summary>
        /// Defines the caller for System.Math.Sqrt(float) -> float
        /// </summary>
        public static MethodReference Sqrt = new MethodReference(TypeReference.Float64, "System.Math.Sqrt", TypeReference.Float64, [TypeReference.Float64]);

    }
}
