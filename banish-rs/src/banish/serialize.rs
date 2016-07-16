use std::io::{Write, Read};

pub trait Serialize {
	fn write(&self, out: &mut Write);
	fn read(r: &mut Read) -> Self;
}