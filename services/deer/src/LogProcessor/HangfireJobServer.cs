using System;
using Hangfire;

namespace LogProcessor
{
    // TODO use
    public class HangfireJobServer : IDisposable
    {
        private readonly BackgroundJobServer _server;

        public HangfireJobServer(JobStorage storage)
        {
            _server = new BackgroundJobServer(storage);
        }

        public void Dispose()
        {
            _server?.Dispose();
        }
    }
}