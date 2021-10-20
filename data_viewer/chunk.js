let chunks = {};

class Chunk {
	constructor(id, offset, scale, size) {
		this.id = id;
		this.offset = offset;
		this.scale = scale;
		this.size = size;
		this.data = {};
	}

	getPointBuffer(gl, key, density_key) {
		if (density_key) {
			let buffer_array = [];
			let data = this.data[key];
			let density_data = this.data[density_key];
			for (let i = 0; i < this.size; ++i) {
				if (density_data[i] != 0) {
					buffer_array.push(i * this.scale + this.offset);
					buffer_array.push(data[i]);
				}
			}
			let buffer_data = new Float32Array(buffer_array);
			let buffer = createBuffer(gl, buffer_data);
			buffer.vertex_count = buffer_array.length / 2;
			return buffer;
		} else {
			let buffer_data = new Float32Array(this.size * 2);
			let data = this.data[key];
			for (let i = 0; i < this.size; ++i) {
				buffer_data[i * 2] = i * this.scale + this.offset;
				buffer_data[i * 2 + 1] = data[i];
			}
			let buffer = createBuffer(gl, buffer_data);
			buffer.vertex_count = this.size;
			return buffer;
		}
	}

	getLineBuffer(gl, key, density_key) {
		if (density_key) {

			let strokes = 0;
			let x;
			let y;

			let buffer_array = [];
			let data = this.data[key];
			let density_data = this.data[density_key];
			for (let i = 0; i < this.size; ++i) {
				if (density_data[i] != 0) {
					let nx = i * this.scale + this.offset;
					let ny = data[i];
					if (strokes > 0) {
						buffer_array.push(x);
						buffer_array.push(y);
						buffer_array.push(nx);
						buffer_array.push(ny);
					}
					++strokes;
					x = nx;
					y = ny;
				} else {
					if (strokes == 1) {
						buffer_array.push(x - 0.5 * this.scale);
						buffer_array.push(y);
						buffer_array.push(x + 0.5 * this.scale);
						buffer_array.push(y);
					}
					strokes = 0;
				}
			}

			if (strokes == 1) {
				buffer_array.push(x - 0.5 * this.scale);
				buffer_array.push(y);
				buffer_array.push(x + 0.5 * this.scale);
				buffer_array.push(y);
			}

			let buffer_data = new Float32Array(buffer_array);
			let buffer = createBuffer(gl, buffer_data);
			buffer.vertex_count = buffer_array.length / 2;
			return buffer;
		} else {
			let buffer_data = new Float32Array(this.size * 2);
			let data = this.data[key];
			for (let i = 0; i < this.size; ++i) {
				buffer_data[i * 2] = i * this.scale + this.offset;
				buffer_data[i * 2 + 1] = data[i];
			}
			let buffer = createBuffer(gl, buffer_data);
			buffer.vertex_count = this.size;
			return buffer;
		}
	}

	getAreaBuffer(gl, key, density_key) {
		if (density_key) {

			let strokes = 0;
			let x;
			let y;

			let buffer_array = [];
			let data = this.data[key];
			let density_data = this.data[density_key];
			for (let i = 0; i < this.size; ++i) {
				if (density_data[i] != 0) {
					let nx = i * this.scale + this.offset;
					let ny = data[i];
					if (strokes > 0) {
						buffer_array.push(x);
						buffer_array.push(y);
						buffer_array.push(x);
						buffer_array.push(0.0);
						buffer_array.push(nx);
						buffer_array.push(ny);
						buffer_array.push(x);
						buffer_array.push(0.0);
						buffer_array.push(nx);
						buffer_array.push(0.0);
						buffer_array.push(nx);
						buffer_array.push(ny);
					}
					++strokes;
					x = nx;
					y = ny;
				} else {
					if (strokes == 1) {
						buffer_array.push(x - 0.5 * this.scale);
						buffer_array.push(y);
						buffer_array.push(x - 0.5 * this.scale);
						buffer_array.push(0.0);
						buffer_array.push(x + 0.5 * this.scale);
						buffer_array.push(y);
						buffer_array.push(x - 0.5 * this.scale);
						buffer_array.push(0.0);
						buffer_array.push(x + 0.5 * this.scale);
						buffer_array.push(0.0);
						buffer_array.push(x + 0.5 * this.scale);
						buffer_array.push(y);
					}
					strokes = 0;
				}
			}

			if (strokes == 1) {
				buffer_array.push(x - 0.5 * this.scale);
				buffer_array.push(y);
				buffer_array.push(x - 0.5 * this.scale);
				buffer_array.push(0.0);
				buffer_array.push(x + 0.5 * this.scale);
				buffer_array.push(y);
				buffer_array.push(x - 0.5 * this.scale);
				buffer_array.push(0.0);
				buffer_array.push(x + 0.5 * this.scale);
				buffer_array.push(0.0);
				buffer_array.push(x + 0.5 * this.scale);
				buffer_array.push(y);
			}

			let buffer_data = new Float32Array(buffer_array);
			let buffer = createBuffer(gl, buffer_data);
			buffer.vertex_count = buffer_array.size / 2;
			return buffer;
		} else {
			let buffer_data = new Float32Array(this.size * 4);
			let data = this.data[key];
			for (let i = 0; i < this.size; ++i) {
				let x = i * this.scale + this.offset;
				buffer_data[i * 4] = x;
				buffer_data[i * 4 + 1] = data[i];
				buffer_data[i * 4 + 2] = x;
				buffer_data[i * 4 + 3] = 0.0;
			}
			let buffer = createBuffer(gl, buffer_data);
			buffer.vertex_count = this.size * 2;
			return buffer;
		}
	}

	getRectangleBuffer(gl, keys, density_key) {
		let positions = [];
		let colors = [];
		let vertex_count = 0;
		let data_lines = keys.map(key => this.data[key]);
		let density_data = density_key && this.data[density_key];
		for (let i = 0; i < this.size; ++i) {
			if (!density_data || density_data[i] != 0) {
				let x = i * this.scale + this.offset;
				let nx = (i + 1) * this.scale + this.offset;
				
				positions.push(x);
				positions.push(1.0);
				positions.push(x);
				positions.push(0.0);
				positions.push(nx);
				positions.push(1.0);
				positions.push(x);
				positions.push(0.0);
				positions.push(nx);
				positions.push(0.0);
				positions.push(nx);
				positions.push(1.0);

				vertex_count += 6;

				for (let j = 0; j < 6; ++j) {
					for (const data of data_lines) {
						colors.push(data[i] * 255);
					}
				}
			}
		}

		let array_buffer = new ArrayBuffer(positions.length * 4 + colors.length);

		const position_view = new Float32Array(array_buffer, 0, positions.length);
		position_view.set(positions);

		const color_view = new Uint8Array(array_buffer, positions.length * 4, colors.length);
		color_view.set(colors);

		let buffer = createBuffer(gl, array_buffer);
		buffer.offset = {
			position: 0,
			color: positions.length * 4
		};
		buffer.vertex_count = vertex_count;
		return buffer;
	}
}

function get_chunk(chr, size, index) {
	let chunk_identifier = chr + "_" + size + "_" + index;
	//console.log(chunk_identifier);
	if (chunk_identifier in chunks) {
		return chunks[chunk_identifier];
	} else {
		chunks[chunk_identifier] = null;
		fetch_chunk(chr, size, index);
		return null;
	}
}

function fetch_chunk(chr, size, index) {
	let chunk_identifier = chr + "_" + size + "_" + index;
	fetch(chunk_path + chunk_identifier + ".cvd")
	.then(response => {
		if (response.ok)
			return response.arrayBuffer()
		else
			return null;
	})
	.then(arrayBuffer => {
		if (arrayBuffer === null)
			return;

		let chunk_offset = chromosome_offsets[chromosome_names.findIndex(e => e == chr)] + size * index * chunk_size;
		let chunk_size_cutoff = (chromosome_sizes[chromosome_names.findIndex(e => e == chr)] - size * index * chunk_size) / size;

		let chunk = new Chunk(chunk_identifier, chunk_offset, size, Math.min(chunk_size, chunk_size_cutoff));
		chunks[chunk_identifier] = chunk;

		let offset = 0;
		for (const data_line of chunk_data_lines) {
			chunk.data[data_line] = new Float32Array(arrayBuffer, offset, chunk.size);
			offset += chunk_size * 4;
		}

		// console.log(chunk);

		render_out_of_date = true;

		// console.log(chunk_identifier);
	});
}
