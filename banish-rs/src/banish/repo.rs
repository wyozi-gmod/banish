use banish::Commit;

pub struct Repository<T> {
	head: Option<Commit<T>>
}

impl <T> Repository<T> {
	pub fn new(head: Commit<T>) -> Repository<T> {
		Repository {
			head: Some(head)
		}
	}

	pub fn new_empty() -> Repository<T> {
		Repository { head: None }
	}

	pub fn update_head(&mut self, new_head: Commit<T>) {
		self.head = Some(new_head);
	}

	pub fn get_head(&self) -> Option<&Commit<T>> {
		self.head.as_ref()
	}
}