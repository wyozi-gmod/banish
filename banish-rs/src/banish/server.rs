extern crate hyper;

use banish::{Repository, Serialize};

use self::hyper::server::{Server, Listening, Handler, Request, Response};
use std::io::Write;
use std::sync::Arc;

struct BanishHandler<T: 'static + Send + Sync + Serialize> {
	repo: Arc<Box<Repository<T>>>,
}
impl<T: 'static + Send + Sync + Serialize> Handler for BanishHandler<T> {
    fn handle(&self, req: Request, res: Response) {
		let head = self.repo.get_head();

		match head {
			Some(commit) => {
				let mut res = res.start().unwrap();
				commit.data.write(&mut res);
			}
			_ => {
				res.send(b"no commit").unwrap();
			}
		}
		
		//res.headers_mut().set(ContentLength(body.len() as u64));
    }
}

pub struct BanishServer<T> {
	repo: Arc<Box<Repository<T>>>,
	listen_server: Listening
}

impl <T> BanishServer<T> {
	pub fn stop(&mut self) {
		self.listen_server.close();
	}
}

pub fn create<T: 'static + Send + Sync + Serialize>(repo: &Arc<Box<Repository<T>>>, port: u16) -> BanishServer<T> {
	let hyper_server = Server::http(("0.0.0.0", port)).unwrap();

	let handler = BanishHandler { repo: repo.clone() };
	let listening = hyper_server.handle(handler).unwrap();

	BanishServer {
		repo: repo.clone(),
		listen_server: listening
	}
}