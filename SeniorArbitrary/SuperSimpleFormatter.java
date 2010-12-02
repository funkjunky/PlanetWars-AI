import java.util.Date;
import java.util.logging.*;

public class SuperSimpleFormatter extends SimpleFormatter {
	public String format(LogRecord record) {
		Date time = new java.util.Date();
		time.setTime(record.getMillis());
		String timestamp = time.toString();
		return timestamp + ": " + record.getMessage() + "\n";
	}
}
