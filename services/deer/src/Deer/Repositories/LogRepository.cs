using System;
using System.Security.Cryptography.X509Certificates;
using System.Threading.Tasks;
using Deer.Messages;
using Elasticsearch.Net;
using Nest;

namespace Deer.Repositories
{
    public class LogRepository : ILogRepository
    {
        private ElasticClient _elasticClient;

        public LogRepository(Uri elasticSearchUri)
        {
            var userInfo = elasticSearchUri.UserInfo.Split(':');
            var user = userInfo[0];
            var password = userInfo[1];

            var rootCaCert = new X509Certificate2("root-ca.pem");

            var nestSettings = new ConnectionSettings(elasticSearchUri)
                .BasicAuthentication(user, password)
                .ServerCertificateValidationCallback(CertificateValidations.AuthorityIsRoot(rootCaCert));
            
            _elasticClient = new ElasticClient(nestSettings);
        }

        public async Task IndexAsync(string userName, LogData logData)
        {
            await _elasticClient.IndexAsync(logData, id => id.Index(userName));
        }
    }
}