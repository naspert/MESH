
/* Bitmask for the midpoint search */
#define U0_FOUND 0x01
#define U1_FOUND 0x02
#define U2_FOUND 0x04

#ifdef __cplusplus
extern "C" {
#endif

  struct model* subdiv(struct model*, 
		       void (*midpoint_func)(struct ring_info*, int, int, 
					     struct model*, vertex_t*), 
                        void (*midpoint_func_bound)(struct ring_info*, int, 
                                                    int, struct model*, 
                                                    vertex_t*), 
		       void (*update_func)(struct model*, struct model*, 
					   struct ring_info*));

#ifdef __cplusplus
}
#endif
