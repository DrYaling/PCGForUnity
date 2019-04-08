
public static class EgineUtils
{
    static int instanceId = 0;
    public static int GetInstanceId()
    {
        if (instanceId == int.MaxValue)
            throw new System.Exception("too much instance was created");
        return instanceId++;
    }
}