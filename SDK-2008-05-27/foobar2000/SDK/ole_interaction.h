class NOVTABLE playlist_dataobject_desc {
public:
	virtual t_size get_entry_count() const = 0;
	virtual void get_entry_name(t_size which, pfc::string_base & out) const = 0;
	virtual void get_entry_content(t_size which, metadb_handle_list_ref out) const = 0;

	virtual void set_entry_count(t_size count) = 0;
	virtual void set_entry_name(t_size which, const char * name) = 0;
	virtual void set_entry_content(t_size which, metadb_handle_list_cref content) = 0;

	void copy(playlist_dataobject_desc const & source) {
		const t_size count = source.get_entry_count();  set_entry_count(count);
		metadb_handle_list content; pfc::string8 name;
		for(t_size walk = 0; walk < count; ++walk) {
			source.get_entry_name(walk,name); source.get_entry_content(walk,content);
			set_entry_name(walk,name); set_entry_content(walk,content);
		}
	}

	void set_from_playlist_manager(bit_array const & mask) {
		static_api_ptr_t<playlist_manager> api;
		const t_size pltotal = api->get_playlist_count();
		const t_size total = mask.calc_count(true,0,pltotal);
		set_entry_count(total);
		t_size done = 0;
		pfc::string8 name; metadb_handle_list content;
		for(t_size walk = 0; walk < pltotal; ++walk) if (mask[walk]) {
			pfc::dynamic_assert( done < total );
			api->playlist_get_name(walk,name); api->playlist_get_all_items(walk,content);
			set_entry_name(done,name); set_entry_content(done,content);
			++done;
		}
		pfc::dynamic_assert( done == total );
	}
	
	const playlist_dataobject_desc& operator=(const playlist_dataobject_desc& source) {copy(source); return *this;}
protected:
	~playlist_dataobject_desc() {}
};

class playlist_dataobject_desc_impl : public playlist_dataobject_desc {
public:
	playlist_dataobject_desc_impl() {}
	playlist_dataobject_desc_impl(const playlist_dataobject_desc & source) {copy(source);}

	t_size get_entry_count() const {return m_entries.get_size();}
	void get_entry_name(t_size which, pfc::string_base & out) const {
		if (which < m_entries.get_size()) out = m_entries[which].m_name;
		else throw pfc::exception_invalid_params();
	}
	void get_entry_content(t_size which, metadb_handle_list_ref out) const {
		if (which < m_entries.get_size()) out = m_entries[which].m_content;
		else throw pfc::exception_invalid_params();
	}
	void set_entry_count(t_size count) {
		m_entries.set_size(count);
	}
	void set_entry_name(t_size which, const char * name) {
		if (which < m_entries.get_size()) m_entries[which].m_name = name;
		else throw pfc::exception_invalid_params();
	}
	void set_entry_content(t_size which, metadb_handle_list_cref content) {
		if (which < m_entries.get_size()) m_entries[which].m_content = content;
		else throw pfc::exception_invalid_params();
	}
private:
	struct entry { metadb_handle_list m_content; pfc::string8 m_name; };
	pfc::array_t<entry> m_entries;
};

//! New in 0.9.5. Provides various methods for interaction between foobar2000 and OLE IDataObjects, Windows Clipboard, drag&drop and such.
//! To instantiate, use static_api_ptr_t<ole_interaction>.
class ole_interaction : public service_base {
public:
	enum {
		KClipboardFormatSimpleLocations,
		KClipboardFormatFPL,
		KClipboardFormatMultiFPL,
		KClipboardFormatTotal
	};
	//! Retrieves clipboard format ID for one of foobar2000's internal data formats.
	//! @param which One of KClipboardFormat* constants.
	virtual t_uint32 get_clipboard_format(t_uint32 which) = 0;

	//! Creates an IDataObject from a group of tracks.
	virtual pfc::com_ptr_t<IDataObject> create_dataobject(metadb_handle_list_cref source) = 0;
	
	//! Creates an IDataObject from one or more playlists, including playlist name info for re-creating those playlists later.
	virtual pfc::com_ptr_t<IDataObject> create_dataobject(const playlist_dataobject_desc & source) = 0;

	//! Attempts to parse an IDataObject as playlists.
	virtual HRESULT parse_dataobject_playlists(pfc::com_ptr_t<IDataObject> obj, playlist_dataobject_desc & out) = 0;

	//! For internal use only. Will succeed only if the metadb_handle list can be generated immediately, without performing potentially timeconsuming tasks such as parsing media files (for an example when the specified IDataObject contains data in one of our internal formats).
	virtual HRESULT parse_dataobject_immediate(pfc::com_ptr_t<IDataObject> obj, metadb_handle_list_ref out) = 0;

	//! Attempts to parse an IDataObject into a dropped_files_data object (list of metadb_handles if immediately available, list of file paths otherwise).
	virtual HRESULT parse_dataobject(pfc::com_ptr_t<IDataObject> obj, dropped_files_data & out) = 0;

	//! Checks whether the specified IDataObject appears to be parsable by our parse_dataobject methods.
	virtual HRESULT check_dataobject(pfc::com_ptr_t<IDataObject> obj, DWORD & dropEffect, bool & isNative) = 0;

	//! Checks whether the specified IDataObject appears to be parsable as playlists (parse_dataobject_playlists method).
	virtual HRESULT check_dataobject_playlists(pfc::com_ptr_t<IDataObject> obj) = 0;

	FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(ole_interaction)
};
