pub struct Commit<T> {
	pub data: Box<T>
}

impl <T> Commit<T> {
	pub fn new(data: T) -> Commit<T> {
		Commit { data: Box::new(data) }
	}
}