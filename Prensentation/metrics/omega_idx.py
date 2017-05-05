def calc_omega_idx(num_vertices, result_comm_list, ground_truth_comm_list):
    return OmegaIdx(num_vertices, result_comm_list, ground_truth_comm_list).calculate_omega_idx()


class OmegaIdx:
    def __init__(self, num_vertices, result_comm_list, ground_truth_comm_list):
        self.result_comm_list = map(sorted, result_comm_list)
        self.ground_truth_comm_list = map(sorted, ground_truth_comm_list)
        self.num_vertex_pairs = num_vertices * (num_vertices - 1) / 2

    def calculate_omega_idx(self):
        def get_pair_count_dict(comm_list):
            pair_count_dict = {}
            for comm in comm_list:
                for i in xrange(len(comm)):
                    for j in xrange(i, len(comm)):
                        if (comm[i], comm[j]) not in pair_count_dict:
                            pair_count_dict[comm[i], comm[j]] = 0
                        pair_count_dict[comm[i], comm[j]] += 1
            return pair_count_dict

        def reverse_dict(original_dict):
            reversed_dict = {}
            for key, val in original_dict:
                if val not in reversed_dict:
                    reversed_dict[val] = set()
                reversed_dict[val].add(key)
            return reversed_dict

        result_pair_list_dict = reverse_dict(get_pair_count_dict(self.result_comm_list))
        ground_truth_pair_list_dict = reverse_dict(get_pair_count_dict(self.ground_truth_comm_list))

        def cal_unadjusted_val():
            whole_count = 0
            for count in result_pair_list_dict:
                if count in ground_truth_pair_list_dict:
                    whole_count += len(result_pair_list_dict[count] & ground_truth_pair_list_dict[count])
            return float(whole_count) / self.num_vertex_pairs

        def cal_expected_val():
            whole_count = 0
            for count in result_pair_list_dict:
                if count in ground_truth_pair_list_dict:
                    whole_count += len(result_pair_list_dict[count]) * len(ground_truth_pair_list_dict[count])
            return float(whole_count) / (self.num_vertex_pairs ** 2)

        unadjusted_val = cal_unadjusted_val()
        expected_val = cal_expected_val()
        return (unadjusted_val - expected_val) / (1 - expected_val)


if __name__ == '__main__':
    print calc_omega_idx(9, [[0, 1, 5], [1, 2, 3, 7, 4, 8]], [[0, 1, 2, 3, 4, 5, 7, 8], [0, 5]])
