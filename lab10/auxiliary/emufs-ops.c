#include "emufs-disk.h"
#include "emufs.h"

/* ------------------- In-Memory objects ------------------- */

int init=0; //if the file/directory handles arrays are initialized or not

struct file_t
{
	int offset;		                // offset of the file
	int inode_number;	            // inode number of the file in the disk
	int mount_point;    			// reference to mount point
                                    // -1: Free
                                    // >0: In Use
};

struct directory_t
{
    int inode_number;               // inode number of the directory in the disk
    int mount_point;    			// reference to mount point
                                    // -1: Free
                                    // >0: In Use
};


struct directory_t dir[MAX_DIR_HANDLES];    // array of directory handles
struct file_t files[MAX_FILE_HANDLES];      // array of file handles

int closedevice(int mount_point){
    /*
        * Close all the associated handles
        * Unmount the device
        
        * Return value: -1,     error
                         1,     success
    */

    for(int i=0; i<MAX_DIR_HANDLES; i++)
        dir[i].mount_point = (dir[i].mount_point==mount_point ? -1 : dir[i].mount_point);
    for(int i=0; i<MAX_FILE_HANDLES; i++)
        files[i].mount_point = (files[i].mount_point==mount_point ? -1 : files[i].mount_point);
    
    return closedevice_(mount_point);
}

int create_file_system(int mount_point, int fs_number){
    /*
	   	* Read the superblock.
        * Update the mount point with the file system number
	    * Set file system number on superblock
		* Clear the bitmaps.  values on the bitmap will be either '0', or '1'. 
        * Update the used inodes and blocks
		* Create Inode 0 (root) in metadata block in disk
		* Write superblock and metadata block back to disk.

		* Return value: -1,		error
						 1, 	success
	*/
    struct superblock_t superblock;
    read_superblock(mount_point, &superblock);

    update_mount(mount_point, fs_number);

    superblock.fs_number=fs_number;
    for(int i=3; i<MAX_BLOCKS; i++)
        superblock.block_bitmap[i]=0;
    for(int i=0; i<3; i++)
        superblock.block_bitmap[i]=1;
    for(int i=1; i<MAX_INODES; i++)
        superblock.inode_bitmap[i]=0;
    superblock.inode_bitmap[0]=1;
    superblock.used_blocks=3;
    superblock.used_inodes=1;
    write_superblock(mount_point, &superblock);

    struct inode_t inode;
    memset(&inode,0,sizeof(struct inode_t));
    inode.name[0]='/';
    inode.parent=255;
    inode.type=1;
    write_inode(mount_point, 0, &inode);
}

int alloc_dir_handle(){
    /*
        * Initialize the arrays if not already done
        * check and return if there is any free entry
        
		* Return value: -1,		error
						 1, 	success
    */
    if(init==0){
        for(int i=0; i<MAX_DIR_HANDLES; i++)
            dir[i].mount_point = -1;
        for(int i=0; i<MAX_FILE_HANDLES; i++)
            files[i].mount_point = -1;
        init=1;
    }
    for(int i=0; i<MAX_DIR_HANDLES; i++)
        if(dir[i].mount_point==-1)
            return i;
    return -1;
}

int alloc_file_handle(){
    for(int i=0; i<MAX_FILE_HANDLES; i++)
        if(files[i].mount_point==-1)
            return i;
    return -1;
}

int goto_parent(int dir_handle){
    /*
        * Update the dir_handle to point to the parent directory
        
		* Return value: -1,		error   (If the current directory is root)
						 1, 	success
    */

    struct inode_t inode;
    read_inode(dir[dir_handle].mount_point, dir[dir_handle].inode_number, &inode);
    if(inode.parent==255)
        return -1;
    dir[dir_handle].inode_number = inode.parent;
    return 1;
}

int open_root(int mount_point){
    /*
        * Open a directory handle pointing to the root directory of the mount
        
		* Return value: -1,		            error (no free handles)
						 directory handle, 	success
    */

    int free_dir_handle = alloc_dir_handle();       // get a free handle number
    if(free_dir_handle==-1) return -1;
    
    dir[free_dir_handle].inode_number = 0;          // given in PDF: inode 0 is inode for root dir
    dir[free_dir_handle].mount_point = mount_point;

    return free_dir_handle;

}

int return_inode(int mount_point, int inodenum, char* path){
    /*
        * Parse the path 
        * Search the directory to find the matching entity
        
		* Return value: -1,		        error
						 inode number, 	success
    */

    // start from root directory
    if(path[0]=='/')
        inodenum=0;

    struct inode_t inode;
    read_inode(mount_point, inodenum, &inode);

    // the directory to start with is not a directory
    if(inode.type==0)
        return -1;

    int ptr1=0, ptr2=0;
    char buf[MAX_ENTITY_NAME];
    memset(buf,0,MAX_ENTITY_NAME);
    
    while(path[ptr1]){
        if(path[ptr1]=='/'){
            ptr1++;
            continue;
        }
        if(path[ptr1]=='.'){
            ptr1++;
            if(path[ptr1]=='/' || path[ptr1]==0)
                continue;
            if(path[ptr1]=='.'){
                ptr1++;
                if(path[ptr1]=='/' || path[ptr1]==0){
                    if(inodenum==0)
                        return -1;
                    inodenum = inode.parent;
                    read_inode(mount_point, inodenum, &inode);
                    continue;
                }
            }
            return -1;
        }
        while(1){
            int found=0;
            buf[ptr2++]=path[ptr1++];
            if(path[ptr1]==0 || path[ptr1]=='/'){
                for(int i=0; i<inode.size; i++){
                    struct inode_t entry;
                    read_inode(mount_point, inode.mappings[i], &entry);
                    if(memcmp(buf,entry.name,MAX_ENTITY_NAME)==0){
                        inodenum = inode.mappings[i];
                        inode = entry;
                        if(path[ptr1]=='/')
                            if(entry.type==0)
                                return -1;
                        ptr2=0;
                        memset(buf,0,MAX_ENTITY_NAME);
                        found=1;
                        break;
                    }
                }
                if(found)
                    break;
                return -1;
            }
            if(ptr2==MAX_ENTITY_NAME)
                return -1;
        }
    }
    return inodenum;
}

int change_dir(int dir_handle, char* path){
    /*
        * Update the handle to point to the directory denoted by path
        * You should use return_inode function to get the required inode

		* Return value: -1,		error
						 1, 	success
    */
    
    if(dir_handle<0 || dir_handle >= MAX_DIR_HANDLES) return -1;

    // NOTE: return_inode has pass-by-values...

    int tmp_inode_num = return_inode(dir[dir_handle].mount_point, dir[dir_handle].inode_number, path);
    // printf("tmp_inode_num %d\n", tmp_inode_num);
    if(tmp_inode_num==-1) return -1;

    // change this file handle
    dir[dir_handle].inode_number = tmp_inode_num;
    return 1;

}

void emufs_close(int handle, int type){
    /*
        * type = 1 : Directory handle and 0 : File Handle
        * Close the file/directory handle
    */
    
    if(type==1) {
        dir[handle].mount_point = -1;
    }
    else {
        files[handle].mount_point = -1;
    }

}

int delete_entity(int mount_point, int inodenum){
    /*
        * Delete the entity denoted by inodenum (inode number)
        * Close all the handles associated
        * If its a file then free all the allocated blocks
        * If its a directory call delete_entity on all the entities present
        * Free the inode
        
        * Return value : inode number of the parent directory
    */

    struct inode_t inode;
    read_inode(mount_point, inodenum, &inode);
    if(inode.type==0){
        for(int i=0; i<MAX_FILE_HANDLES; i++)
            if(files[i].inode_number==inodenum)
                files[i].mount_point=-1;
        int num_blocks = inode.size/BLOCKSIZE;
        if(num_blocks*BLOCKSIZE<inode.size)
            num_blocks++;
        for(int i=0; i<num_blocks; i++)
            free_datablock(mount_point, inode.mappings[i]);
        free_inode(mount_point, inodenum);
        return inode.parent;
    }

    for(int i=0; i<MAX_DIR_HANDLES; i++)
        if(dir[i].inode_number==inodenum)
            dir[i].mount_point=-1;
    
    for(int i=0; i<inode.size; i++)
        delete_entity(mount_point, inode.mappings[i]);
    free_inode(mount_point, inodenum);
    return inode.parent;
}

int emufs_delete(int dir_handle, char* path){
    /*
        * Delete the entity at the path
        * Use return_inode and delete_entry functions for searching and deleting entities 
        * Update the parent root of the entity to mark the deletion
        * Remove the entity's inode number from the mappings array and decrease the size of the directory
        * For removing entity do the following : 
            * Lets say we have to remove ith entry in mappings
            * We'll make mappings[j-1] = mappings[j] for all i < j < size 
        * Then write the inode back to the disk
        
        * Return value: -1, error
                         1, success
    */
    
    if(dir_handle<0 || dir_handle>=MAX_DIR_HANDLES) return -1;
    
    int mt_pt = dir[dir_handle].mount_point;

    int i_n_to_del = return_inode(mt_pt, dir[dir_handle].inode_number, path);        // inode_num_to_delete
    if(i_n_to_del==-1) return -1;

    struct inode_t curr_inode;                      // *** allocate on stack √
                                                    // otherwise, need to malloc and free
                                                    // (if not done, segfault ofc)
                                                    // (gdb...)
    read_inode(mt_pt, i_n_to_del, &curr_inode);
    // printf("curr_inode_to_del: ");
    // printf("%s %d %d %d ", curr_inode.name, curr_inode.type, curr_inode.parent, curr_inode.size);
    // for(int i=0; i<4; i++) {
    //     printf("%d:%d ", i, curr_inode.mappings[i]);
    // }
    // printf("\n");

    int par_i_n = curr_inode.parent;

    struct inode_t parent;
    read_inode(mt_pt, par_i_n, &parent);

    int i;

    delete_entity(mt_pt, i_n_to_del);

    // for(i=0; i<4; i++) {
    //     printf("old parent mapping %d:%d\n", i, parent.mappings[i]);
    // }

    int map_index_to_del = -1;
    for(i=0; i<parent.size; i++) {
        if(map_index_to_del!=-1) {      // move later mappings by 1 index left
            parent.mappings[i-1] = parent.mappings[i];
            parent.mappings[i] = -1;   // so that last becomes -1...
        }
        if(parent.mappings[i]==i_n_to_del) {
            map_index_to_del = i;
        }
    }
    if(map_index_to_del!=-1) {
        parent.size--;
    }

    write_inode(mt_pt, par_i_n, &parent);       // was missing this... got to know by
                                                // printing parent mappings before and after

    // for(i=0; i<4; i++) {
    //     printf("new parent mapping %d:%d\n", i, parent.mappings[i]);
    // }

    return 1;

}

int emufs_create(int dir_handle, char* name, int type){
    /*
        * Create a directory (type=1) / file (type=0) in the directory denoted by dir_handle
        * Check if a directory/file with the same name is present or not
        * Note that different entities with the same name work, like a file and directory of name 'foo' can exist simultaneously in a directory
        
        * Return value: -1, error
                         1, success
    */

    if(dir_handle<0 || dir_handle>=MAX_DIR_HANDLES) return -1;

    int mt_pt = dir[dir_handle].mount_point;
    int par_i_n = dir[dir_handle].inode_number;
    
    struct inode_t inode_parent;
    read_inode(mt_pt, par_i_n, &inode_parent);

    if(inode_parent.size==4) {
        // 4 entities in parent
        return -1;
    }

    int i;
    struct inode_t tmp_inode_ptr;
    for(i=0; i<inode_parent.size; i++) {        // do NOT iterate over all inodes... (i=0 to MAX_INODES-1)
                                                // because some of them might be uninitialized,
                                                // and return -1
        read_inode(mt_pt, (int)inode_parent.mappings[i], &tmp_inode_ptr);
        if(tmp_inode_ptr.type==type && strcmp(tmp_inode_ptr.name,name)==0) {
            // already present
            return -1;
        }
    }

    int new_inode_num = alloc_inode(dir[dir_handle].mount_point);
    if(new_inode_num==-1) return -1;

    inode_parent.mappings[inode_parent.size] = new_inode_num;
    inode_parent.size++;
    // for(i=0; i<4; i++) {
    //     printf("new_new parent mapping %d:%d\n", i, inode_parent.mappings[i]);
    // }
    write_inode(mt_pt, par_i_n, &inode_parent);          // write parent inode

    struct inode_t* new_inode_ptr = (struct inode_t*)malloc(sizeof(struct inode_t));        // malloc new stuff
    strcpy(new_inode_ptr->name, name);
    new_inode_ptr->type = type;
    new_inode_ptr->parent = dir[dir_handle].inode_number;
    new_inode_ptr->size = 0;

    for(i=0; i<4; i++) {
        new_inode_ptr->mappings[i] = -1;
        // printf("%d\n", new_inode_ptr->mappings[i]);
    }

    // write new inode
    write_inode(mt_pt, new_inode_num, new_inode_ptr);

    free(new_inode_ptr);        // free malloc'd space

    return 1;

}

int open_file(int dir_handle, char* path){
    /*
        * Open a file_handle to point to the file denoted by path
        * Get the inode using return_inode function
        * Get a file handle using alloc_file_handle
        * Initialize the file handle
        
        * Return value: -1, error
                         1, success     // CHANGED -- returning file_handle_no
    */

    int free_file_handle = alloc_file_handle();
    // printf("free_file_handle %d\n", free_file_handle);
    if(free_file_handle==-1) return -1;     // was doing "=-1" -- got to know by -Wall -Wextra

    files[free_file_handle].inode_number = return_inode(dir[dir_handle].mount_point, dir[dir_handle].inode_number, path);
    // printf("free_file_handle inode_num %d\n", files[free_file_handle].inode_number);
    if(files[free_file_handle].inode_number==-1) {
        // mount_point still -1...
        return -1;
    }
    files[free_file_handle].mount_point = dir[dir_handle].mount_point;
    files[free_file_handle].offset = 0;

    return free_file_handle;

}

int emufs_read(int file_handle, char* buf, int size){
    /*
        * Read the file into buf starting from seek(offset) 
        * The size of the chunk to be read is given
        * size can and can't be a multiple of BLOCKSIZE
        * Update the offset = offset+size in the file handle (update the seek)
        * Hint: 
            * Use a buffer of BLOCKSIZE and read the file blocks in it
            * Then use this buffer to populate buf (use memcpy)
        
        * Return value: -1, error
                         1, success
    */

    if(file_handle<0 || file_handle>=MAX_FILE_HANDLES) return -1;

    int mt_pt = files[file_handle].mount_point;
    int i_n = files[file_handle].inode_number;
    struct inode_t curr_inode;
    read_inode(mt_pt, i_n, &curr_inode);     // void...
    
    int curr_offset = files[file_handle].offset;
    int final_offset = curr_offset+size;
    // int offset_buf = 0;
    // char block_buf[BLOCKSIZE];
    
    // int i;
    // for(i=0; i<4; i++) {
    //     if(curr_inode.mappings[i]==-1) return -1;      // ensured in emufs_delete that
    //                                                     // -1's are at the end
    //     read_datablock(mt_pt, curr_inode.mappings[i], block_buf);      // void
    //     if(curr_offset < (i+1)*BLOCKSIZE) {             // start or continue in this block
    //         if(final_offset < (i+1)*BLOCKSIZE) {        // end in this block
    //             memcpy(buf+offset_buf, block_buf+(curr_offset-i*BLOCKSIZE), final_offset-curr_offset);
    //             offset_buf += (final_offset-curr_offset);
    //             curr_offset = final_offset;
    //             return 1;
    //         }
    //         else {
    //             memcpy(buf, block_buf+(curr_offset-i*BLOCKSIZE), (i+1)*BLOCKSIZE-curr_offset);
    //             offset_buf += (i+1)*BLOCKSIZE-curr_offset;
    //             curr_offset = (i+1)*BLOCKSIZE;      // start of next block
    //         }
    //     }
    // }

    // above M works with a single buffer of BLOCKSIZE
    // could also do: read entire (1024) bytes at once ->

    char buf_all_blocks[4*BLOCKSIZE];
    int i;
    int data_read = 0;
    for(i=0; i<4; i++) {
        if(curr_inode.mappings[i]==-1) break;
        read_datablock(mt_pt, curr_inode.mappings[i], buf_all_blocks+data_read);
        data_read += BLOCKSIZE;
    }
    if(final_offset>data_read) return -1;
    memcpy(buf, buf_all_blocks+curr_offset, size);

    files[file_handle].offset += size;

    return 1;

}

int emufs_write(int file_handle, char* buf, int size){
    /*
        * Write the memory buffer into file starting from seek(offset) 
        * The size of the chunk to be written is given
        * size can and can't be a multiple of BLOCKSIZE
        * Update the inode of the file if need to be (mappings and size changed)
        * Update the offset = offset+size in the file handle (update the seek)
        * Hint: 
            * Use a buffer of BLOCKSIZE and read the file blocks in it
            * Then write to this buffer from buf (use memcpy)
            * Then write back this buffer to the file
        
        * Return value: -1, error
                         1, success
    */
    
    if(file_handle<0 || file_handle>=MAX_FILE_HANDLES) return -1;

    int mt_pt = files[file_handle].mount_point;
    int i_n = files[file_handle].inode_number;
    struct inode_t curr_inode;
    read_inode(mt_pt, i_n, &curr_inode);     // void
    
    int curr_offset = files[file_handle].offset;
    int final_offset = curr_offset+size;
    if(final_offset>4*BLOCKSIZE) return -1;     // usual write() returns no. of bytes written
                                                // vs here

    char buf_all_blocks[4*BLOCKSIZE];
    int i;
    int data_read = 0;
    for(i=0; i<4; i++) {
        // printf("%s, curr_inode.mappings[%d] = %d\n", curr_inode.name, i, curr_inode.mappings[i]);
        if(curr_inode.mappings[i]==-1) break;
        read_datablock(mt_pt, curr_inode.mappings[i], buf_all_blocks+data_read);
        data_read += BLOCKSIZE;
    }

    int tmp_block_num;
    while(final_offset>data_read) {         // while
        // allocate new data block(s)
        tmp_block_num = alloc_datablock(mt_pt);
        if(tmp_block_num==-1) {
            return -1;          // not enuf space!
        }
        curr_inode.mappings[(curr_inode.size)/BLOCKSIZE] = tmp_block_num;
        curr_inode.size += BLOCKSIZE;
        data_read += BLOCKSIZE;
    }

    memcpy(buf_all_blocks+curr_offset, buf, size);

    // now copy from buf_all_blocks to datablocks
    // data_read = valid_datablocks*BLOCKSIZE
    for(i=0; i<data_read/BLOCKSIZE; i++) {
        write_datablock(mt_pt, curr_inode.mappings[i], buf_all_blocks+i*BLOCKSIZE);
    }

    write_inode(mt_pt, i_n, &curr_inode);

    files[file_handle].offset += size;

    return 1;

}

int emufs_seek(int file_handle, int nseek){
    /*
        * Update the seek(offset) of fie handle
        * Make sure its not negative and not exceeding the file size
        
        * Return value: -1, error
                         1, success
    */

    if(file_handle<0 || file_handle>=MAX_FILE_HANDLES) return -1;

    int mt_pt = files[file_handle].mount_point;
    int i_n = files[file_handle].inode_number;
    struct inode_t curr_inode;
    read_inode(mt_pt, i_n, &curr_inode);     // void
    
    int curr_offset = files[file_handle].offset;
    int final_offset = curr_offset + nseek;     // nseek can be neg. too (testcase1)
    if(final_offset<0 || final_offset>=curr_inode.size) {  // ==size too is outside the valid range [0,size-1]
        return -1;
    }

    files[file_handle].offset = final_offset;
    return 1;

}

void flush_dir(int mount_point, int inodenum, int depth){
    /*
        * Print the directory structure of the device
    */

    struct inode_t inode;
    read_inode(mount_point, inodenum, &inode);

    for(int i=0; i<depth-1; i++)
        printf("|  ");
    if(depth)
        printf("|--");
    for(int i=0; i<MAX_ENTITY_NAME && inode.name[i]>0; i++)
        printf("%c",inode.name[i]);
    if(inode.type==0)
        printf(" (%d bytes)\n", inode.size);
    else{
        printf("\n");
        for(int i=0; i<inode.size; i++)
            flush_dir(mount_point, inode.mappings[i], depth+1);
    }
}

void fsdump(int mount_point)
{
    /*
        * Prints the metadata of the file system
    */
   
    struct superblock_t superblock;
    read_superblock(mount_point, &superblock);
    printf("\n[%s] fsdump \n", superblock.device_name);
    flush_dir(mount_point, 0, 0);
    printf("Inodes in use: %d, Blocks in use: %d\n",superblock.used_inodes, superblock.used_blocks);
}