/**
 * tiny-AESベンチマーク結果
 */
public class BenchmarkResult {

    /** データサイズ(Byte) */
    private long dataSize;

    /** 繰り返し回数 */
    private int iterations;

    /** 合計実行時間(秒) */
    private double totalTime;

    /** 平均実行時間(秒) */
    private double averageTime;

    /** スループット(MiBit/s) */
    private double throughput;

    public BenchmarkResult() {
    }

    public long getDataSize() {
        return dataSize;
    }

    public void setDataSize(long dataSize) {
        this.dataSize = dataSize;
    }

    public int getIterations() {
        return iterations;
    }

    public void setIterations(int iterations) {
        this.iterations = iterations;
    }

    public double getTotalTime() {
        return totalTime;
    }

    public void setTotalTime(double totalTime) {
        this.totalTime = totalTime;
    }

    public double getAverageTime() {
        return averageTime;
    }

    public void setAverageTime(double averageTime) {
        this.averageTime = averageTime;
    }

    public double getThroughput() {
        return throughput;
    }

    public void setThroughput(double throughput) {
        this.throughput = throughput;
    }

    @Override
    public String toString() {
        return String.format(
                "DataSize=%d Bytes, Iterations=%d, Total=%.8f sec, Average=%.8f sec, Throughput=%.3f MiBit/s",
                dataSize,
                iterations,
                totalTime,
                averageTime,
                throughput);
    }
}
