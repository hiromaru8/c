import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

public class TinyAESBenchmark {

    /** JNIライブラリのロード */
    static {
        System.loadLibrary("TinyAESBenchmarkJNI");
    }

    /**
     * tiny_aes のベンチマークを実行する
     *
     * @param dllPath     tiny_aes DLLのパス
     * @param dataSize    データサイズ(Byte)
     * @param iterations  繰り返し回数
     * @return ベンチマーク結果
     */
    public native BenchmarkResult benchmark(
            String dllPath,
            long dataSize,
            int iterations);

    /**
     * CSVへ結果を保存する
     */
    public static void writeCsv(
            String filename,
            BenchmarkResult result)
            throws IOException {

        File file = new File(filename);

        boolean writeHeader =
                !file.exists() || file.length() == 0;

        try (PrintWriter pw =
                     new PrintWriter(
                             new FileWriter(file, true))) {

            if (writeHeader) {
                pw.println(
                        "DataSize(Bytes),Iterations,"
                      + "TotalTime(sec),Average(sec),"
                      + "Throughput(MiBit/s)");
            }

            pw.printf(
                    "%d,%d,%.8f,%.8f,%.3f%n",
                    result.getDataSize(),
                    result.getIterations(),
                    result.getTotalTime(),
                    result.getAverageTime(),
                    result.getThroughput());
        }
    }

    public static void main(String[] args) {

        TinyAESBenchmark bench =
                new TinyAESBenchmark();

        long[] sizes = {
                1024L,
                1024L * 1024L,
                10L * 1024L * 1024L
        };

        String dllPath = "libtiny_aes.dll";

        for (long size : sizes) {

            BenchmarkResult result =
                    bench.benchmark(
                            dllPath,
                            size,
                            100);

            System.out.println(result);

            try {
                writeCsv(
                        "tiny_aes.csv",
                        result);
            }
            catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
