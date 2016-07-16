mod banish;

use banish::{Commit, Serialize};
use std::io::{Read, Write};
use std::sync::Arc;
use std::thread;

extern crate libc;
use libc::*;

pub type BanishServer = libc::c_void;
pub type BanishRepository = libc::c_void;

struct DataPack {
	data: Vec<u8>
}
impl Serialize for DataPack {
	fn write(&self, out: &mut Write) {
		out.write_all(&self.data).unwrap();
	}
	fn read(r: &mut Read) -> Self {
		let mut data = Vec::new();
		r.read_to_end(&mut data).unwrap();
		DataPack { data: data }
	}
}

#[repr(C)]
pub struct BanishData(DataPack);

#[repr(C)]
pub struct BanishDataArray {
	length: usize,
	data: *const libc::c_char
}

#[no_mangle]
pub extern fn banish_create_repo() -> *mut BanishRepository {
	Box::into_raw(Box::new(banish::Repository::<DataPack>::new_empty())) as *mut c_void
}

#[no_mangle]
pub unsafe extern fn banish_pull_from_url(repo_ptr: *mut BanishRepository, url: *const libc::c_char, callback: extern fn(libc::c_int), callback_param: libc::c_int) {
	let url_cstr = std::ffi::CStr::from_ptr(url);

	let remote = banish::Remote::new(url_cstr.to_str().unwrap());

	let repo = &mut *(repo_ptr as *mut banish::Repository<DataPack>);

	thread::spawn(move || {
		remote.fetch_and_merge(repo);
		callback(callback_param);
	});
}

#[no_mangle]
pub unsafe extern fn banish_get_head_data(repo_ptr: *mut BanishRepository, callback: extern fn(libc::c_int, *const BanishDataArray), callback_param: libc::c_int) {
	let repo = &mut *(repo_ptr as *mut banish::Repository<DataPack>);

	thread::spawn(move || {
		match repo.get_head() {
			Some(commit) => {
				let data_vec = &commit.data.data;
				let data_vec_ptr = data_vec.as_ptr();
				let data_vec_charptr = std::mem::transmute::<*const u8, *const libc::c_char>(data_vec_ptr);
				callback(callback_param, Box::into_raw(Box::new(BanishDataArray { length: data_vec.len(), data: data_vec_charptr })))
			}
			_ => callback(callback_param, std::ptr::null())
		}
	});
}

#[no_mangle]
pub unsafe extern fn banish_set_head_data(repo_ptr: *mut BanishRepository, data: *const u8, data_length: usize) {
	let repo = &mut *(repo_ptr as *mut banish::Repository<DataPack>);

	if data_length <= 0 {
		return;
	}

	let mut vec_data = Vec::with_capacity(data_length);
	std::ptr::copy(data, vec_data.as_mut_ptr(), data_length);
	vec_data.set_len(data_length);

	repo.update_head(Commit::new(DataPack { data: vec_data }));
}

#[no_mangle]
pub unsafe extern fn banish_create_server(repo: *mut BanishRepository, port: u16) -> *mut BanishServer {
	let mut repo = Box::from_raw(repo as *mut banish::Repository<DataPack>);
	let local_repo_shared = Arc::new(repo);

	let server = Box::new(banish::server::create(&local_repo_shared, port));
	Box::into_raw(server) as *mut c_void
}

#[no_mangle]
pub unsafe extern fn banish_destroy_server(server: *mut BanishServer) {
	let mut server = Box::from_raw(server as *mut banish::server::BanishServer<String>);
	server.stop()
}