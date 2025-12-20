namespace OCRuntime.Runtime;

public sealed class ManagedObject
{
    public string TypeName { get; }
    private readonly Dictionary<string, object?> _fields = new(StringComparer.Ordinal);

    public ManagedObject(string typeName)
    {
        TypeName = typeName;
    }

    public object? GetField(string name)
    {
        _fields.TryGetValue(name, out var value);
        return value;
    }

    public void SetField(string name, object? value)
    {
        _fields[name] = value;
    }
}
