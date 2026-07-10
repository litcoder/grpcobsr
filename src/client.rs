use std::error::Error;
use tonic::Request;

pub mod stockservice {
    tonic::include_proto!("stockservice");
}

use stockservice::stock_service_client::StockServiceClient;

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    let mut client = StockServiceClient::connect("http://[::1]:50051").await?;

    let mut stream = client
        .update_stock_price(Request::new(()))
        .await?
        .into_inner();

    while let Some(update) = stream.message().await? {
        println!(
            "[{}] {}: {:.2}",
            update.sequence, update.symbol, update.price
        );
    }

    Ok(())
}
