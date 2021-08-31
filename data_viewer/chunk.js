let chunks = {};

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

		let chunk = {};
		chunks[chunk_identifier] = chunk;

		chunk.offset = chromosome_offsets[chromosome_names.findIndex(e => e == chr)] + size * index * chunk_size;
		chunk.data = {};
		chunk.scale = size;

		let chunk_size_cutoff = (chromosome_sizes[chromosome_names.findIndex(e => e == chr)] - size * index * chunk_size) / size;

		let offset = 0;
		for (const data_line of chunk_data_lines) {
			chunk.data[data_line] = new Float32Array(arrayBuffer, offset, Math.min(chunk_size, chunk_size_cutoff));
			offset += chunk_size * 4;
		}

		// console.log(chunk);

		render_out_of_date = true;

		// console.log(chunk_identifier);
	});
}
