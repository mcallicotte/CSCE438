import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;


public class TweetCountTwo {
    public static class TweetMapperTwo extends Mapper<Object, Text, Text, IntWritable> {
        private final static IntWritable one = new IntWritable(1);
        private Text word = new Text();

        public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
            StringTokenizer itr = new StringTokenizer(value.toString());
            //String currentTime = "";
            while (itr.hasMoreTokens()) {
                //System.out.println("Found a time")
                String firstToken = itr.nextToken();
                if (firstToken.equals("T")) {
                    itr.nextToken();
                    StringTokenizer time = new StringTokenizer(itr.nextToken(), ":");
                    //currentTime = time.nextToken();
                    
                    word.set(time.nextToken());
                    // context.write(word, one);
                } else if (firstToken.equals("W")) {
                    while (itr.hasMoreTokens()) {
                        if (itr.nextToken().equalsIgnoreCase("sleep")) {
                            //word.set(currentTime);
                            context.write(word, one);
                            while (itr.hasMoreTokens()) {
                                itr.nextToken();
                            }
                        }
                    }
                } else {
                    while (itr.hasMoreTokens()) {
                        itr.nextToken();
                    }
                }
            }
        }
    }

    public static class TweetReducerTwo extends Reducer<Text,IntWritable,Text,IntWritable> {
        private IntWritable result = new IntWritable();

        public void reduce(Text key, Iterable<IntWritable> values, Context context) throws IOException, InterruptedException {
            int sum = 0;
            for (IntWritable val: values) {
                sum += val.get();
            }
            result.set(sum);
            context.write(key, result);
        }
    }

    public static void main(String[] args) throws Exception {
        Configuration conf = new Configuration();
        Job job = Job.getInstance(conf, "tweet times");
        job.setJarByClass(TweetCountTwo.class);
        job.setMapperClass(TweetMapperTwo.class);
        job.setCombinerClass(TweetReducerTwo.class);
        job.setReducerClass(TweetReducerTwo.class);
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(IntWritable.class);
        FileInputFormat.addInputPath(job, new Path(args[0]));
        FileOutputFormat.setOutputPath(job, new Path(args[1]));
        System.exit(job.waitForCompletion(true) ? 0: 1);
    }
    
}