use futures::StreamExt;
use rand::rngs::SmallRng;
use rand::{Rng, SeedableRng};
use std::pin::Pin;
use std::time::Duration;
use tokio_stream::Stream;
use tonic::{Request, Response, Status, transport::Server};

pub mod stockservice {
    tonic::include_proto!("stockservice");
}

use stockservice::{
    StockPriceResponse,
    stock_service_server::{StockService, StockServiceServer},
};

/// Generate random stock price.
fn generate_stock_price(rng: &mut SmallRng, sequence: u64) -> StockPriceResponse {
    const SYMBOLS: &[&str] = &["AAPL", "GOOG", "MSFT", "AMZN", "TSLA"];
    let symbol = SYMBOLS[rng.random_range(0..SYMBOLS.len())].to_string();
    let price = rng.random_range(1.0..1000.0);

    StockPriceResponse {
        symbol,
        price,
        sequence,
    }
}

// Server 구현체
struct StockServiceImpl;

// tokio_stream을 이용해서 gRPC stream을 구현.
type StockStream = Pin<Box<dyn Stream<Item = Result<StockPriceResponse, Status>> + Send>>;

#[tonic::async_trait]
impl StockService for StockServiceImpl {
    type UpdateStockPriceStream = StockStream;

    async fn update_stock_price(
        &self,
        _request: Request<()>,
    ) -> Result<Response<Self::UpdateStockPriceStream>, Status> {
        // 1초 간격으로 stock price 생성
        let mut interval = tokio_stream::wrappers::IntervalStream::new(tokio::time::interval(
            Duration::from_secs(1),
        ));

        // Stream으로 전송할 데이터를 생성하고 yield로 반환.
        let output = async_stream::stream! {
            let mut rng = SmallRng::from_os_rng();
            let mut sequence: u64 = 0;

            // 매 interval마다 순번을 증가시키고 랜덤 주가를 생성해 클라이언트로 전송
            while interval.next().await.is_some() {
                sequence += 1;
                let p = generate_stock_price(&mut rng, sequence);
                yield Ok(p);
            }
        };

        Ok(Response::new(
            Box::pin(output) as Self::UpdateStockPriceStream
        ))
    }
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let addr = "[::1]:50051".parse()?;
    let service = StockServiceImpl;

    println!("StockService gRPC server listening on {}", addr);

    Server::builder()
        .add_service(StockServiceServer::new(service))
        .serve(addr)
        .await?;

    Ok(())
}
