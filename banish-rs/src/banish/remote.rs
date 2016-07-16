extern crate hyper;
use self::hyper::client::{Client};
use std::io::Read;
use banish::Repository;
use banish::Commit;
use banish::Serialize;

enum RemoteType {
	Url(String),
	File(String)
}

pub struct Remote {
	url: String
}
impl Remote {
	pub fn new(url: &str) -> Remote {
		Remote { url: url.to_owned() }
	}

	pub fn fetch_and_merge<T: Serialize>(&self, repo: &mut Repository<T>) {
		let client = Client::new();
		let mut res = client.get(&self.url).send().unwrap();

		let data = T::read(&mut res);
		let new_commit = Commit::new(data);
		repo.update_head(new_commit);
	}
}