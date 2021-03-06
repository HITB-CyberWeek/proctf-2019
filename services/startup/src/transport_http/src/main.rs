use std::io::{self, Write};
use nix::unistd::alarm;

use hyper::Client;

use hashsum;

const MAX_RESP_SIZE: usize = 100_000;

#[tokio::main]
async fn main() -> Result<(), ()> {
    alarm::set(8);

    let client = Client::new();

    loop {
        let mut input = String::new();
        let url = match io::stdin().read_line(&mut input) {
            Ok(0) => return Ok(()),
            Err(_) => return Err(()),
            Ok(_) => input.trim()
        };

        let url = match url.parse::<hyper::Uri>() {
            Ok(u) => u,
            Err(_) => return Err(())
        };


        let response = match client.get(url.clone()).await {
            Ok(r) => r,
            Err(_) => return Err(())
        };

        let success = response.status().is_success();

        let mut data : Vec<u8> = vec![];
        let mut body = response.into_body();

        while let Some(chunk) = body.next().await {
            match chunk {
                Ok(c) => {
                    if data.len() + c.len() > MAX_RESP_SIZE {
                        return Err(());
                    }
                    data.extend_from_slice(&c)
                },
                Err(_) => return Err(())
            }
        }

        let hashsum = hashsum::calc_hash(&data);

        let headers = format!("Url: {}\nSuccess: {}\nHashsum: {}\nContent-Length: {}\n",
                              url, success, hashsum, data.len());

        if io::stdout().write_all(headers.as_bytes()).is_err() {
            return Err(());
        }

        if io::stdout().write_all(&data).is_err() {
            return Err(());
        }

        if io::stdout().flush().is_err() {
            return Err(());
        }
    }
}
