mod banish;

use banish::Serialize;
use std::io::{Read, Write};
use std::sync::Arc;
impl Serialize for String {
	fn write(&self, out: &mut Write) {
		write!(out,  "{}", self);
	}

	fn read(r: &mut Read) -> Self {
		let mut buf = String::new();
		r.read_to_string(&mut buf);
		buf
	}
}

fn main() {
	let local_commit = banish::Commit::new("xd".to_owned());
	let mut local_repo = banish::Repository::new(local_commit);
	&local_repo.update_head(banish::Commit::new("lol".to_owned()));

	let local_repo_shared = Arc::new(Box::new(local_repo));


	let server = banish::server::create(&local_repo_shared, 8091);
}