namespace LogProcessor.Models
{
    public class User
    {
        public string Username { get; set; }
        
        public string LogIndexName { get; set; }
        
        public string LogExchangeName { get; set; }
        
        public string LogQueueName { get; set; }

        public string ErrorQueueName { get; set; }
    }
}