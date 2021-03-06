#include "pp_encode.h"

pp_encode::pp_encode(std::string FileName)
{
    InputFileName = FileName;

    int i = 0;
    while (FileName[i] != '.')
        ++i;
    FileName = FileName.substr(0, i);
    OutputFileName = FileName + ".pp";
}

/*以下是压缩部分*/
/*The compression part is as below*/

/*int pp_encode::count_each_byte(std::string line, float stat[])
{
    /*use the integer value of a certain byte(like 5 for 00000101) as its id
    ant in this function, we count the frequency of each byte in each line*/
    //unsigned char nl = '\n';
    /*for (int i = 0; i != line.size(); ++i)
    {
    	int byte_num = 0;
    	unsigned char k = 0x80;
    	unsigned char l = line[i];
		stat[(int)l] += 0.01;
		stat[10] += 0.01;
	}
	return 0;
}*/

int pp_encode::get_byte_frequency()
{
    /*this function do the frequency counting job,
    and generate a node array for huffman tree*/
    char in_char;
    std::string buffer, word;

    std::ifstream input_file(InputFileName.c_str(), std::ifstream::binary);
    if (!input_file.is_open())
	{
		std::cout << "Fail to open the file!";
		std::exit(1);
	}
	while (!input_file.eof())
	{
		/*
		std::getline(input_file, buffer);
		count_each_byte(buffer, byte_freq);
		*/
		input_file.get(in_char);
        unsigned char u_char = (unsigned char)in_char;
        ++total_byte;
        if (input_file.eof())
            break;
        byte_freq[(int)u_char] = byte_freq[(int)u_char] + 0.01;
	}
	input_file.close();
	int j = 0;
	for (int i = 0; i < MAX_SIZE; ++i)
    {
        if (byte_freq[i] <= 0)
            continue;
        Node_ptr node = new Huffman_node();
        node->id = i;
        node->freq = byte_freq[i];
        node->code = "";
        node->left = NULL;
        node->parent = NULL;
        node->right = NULL;
        node_array[j++] = node;
    }
    //下面插入伪结束符
    /*Node_ptr node = new Huffman_node();
    node->id = PSEUDO_EOF;
    node->freq = 0.001;
    node->code = "";
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node_array[j++] = node;*/
    size_list = j;
	return 0;
}

int pp_encode::create_pq()
{
    get_byte_frequency();
    for (int i = 0; i < size_list; ++i)
        pq.push(node_array[i]);
    return 0;
}

int pp_encode::build_huffman_tree()
{
    root = NULL;

    while (!pq.empty())
    {
        Node_ptr first = pq.top();
        pq.pop();
        if (pq.empty())
        {
            root = first;
            break;
        }
        Node_ptr second = pq.top();
        pq.pop();
        Node_ptr new_node = new Huffman_node();
        new_node->freq = first->freq + second->freq;

        if (first->freq <= second->freq)
        {
            new_node->left = first;
            new_node->right = second;
        }
        else
        {
            new_node->left = second;
            new_node->right = first;
        }
        first->parent = new_node;
        second->parent = new_node;

        pq.push(new_node);
    }
    return 0;
}

int pp_encode::create_map_table(const Node_ptr node, bool left)
{
    if (left)
        node->code = node->parent->code + "0";
    else
        node->code = node->parent->code + "1";

    // 如果是叶子节点，则是一个“有效”节点，加入编码表
    if (node->left == NULL && node->right == NULL)
        table[node->id] = node->code;
    else
    {
        if (node->left != NULL)
            create_map_table(node->left, true);
        if (node->right != NULL)
            create_map_table(node->right, false);
    }
    return 0;
}

int pp_encode::calculate_huffman_codes()
{
    if (root == NULL)
    {
        std::cout << "No huffman tree exists!";
        std::exit(1);
    }

    if (root->left != NULL)
        create_map_table(root->left, true);
    if (root->right != NULL)
        create_map_table(root->right, false);
    return 0;
}

int pp_encode::do_compress()
{
    std::ifstream input_file(InputFileName.c_str(), std::ifstream::binary);
	std::ofstream output_file(OutputFileName.c_str(), std::ios::out | std::ios::trunc | std::ofstream::binary);
    std::map<int, std::string>::iterator table_it;
    std::string code = "", buffer, out_string;
    int i, j, length;
    char in_char;
    unsigned char out_c, tmp_c;

    output_file << total_byte << std::endl;
    output_file << size_list << std::endl;
    //写入编码表长
    for (table_it = table.begin(); table_it != table.end(); ++table_it)
    {
        output_file << table_it->first << " " << table_it->second << std::endl;
    }
    //写入huffman编码，格式如"43 00100"
    //存入的第一个int可以改成uint8_t试试

    if (!input_file.is_open())
	{
		std::cout << "Error!";
		exit(1);
	}
	/*
    while (!input_file.eof())
    {
        out_string.clear();
        std::getline(input_file, buffer);
        for (int i = 0; i != buffer.size(); ++i)
        {
            unsigned char l = buffer[i];
            table_it = table.find((int)l);
            //std::cout << (int)l << std::endl;
            if (table_it != table.end())
                code += table_it->second;
            else
            {
                printf("Can't find the huffman code of character %X\n", l);
                //exit(1);
            }
        }
        code += table[10];
        //huffman code以二进制流写入到输出文件
        for (i = 0; i + 7 < code.size(); i += 8)
        {
            out_c = 0;
            for (j = 0; j < 8; ++j)
            {
                if ('0' == code[i + j])
                    tmp_c = 0;
                else if ('1' == code[i + j])
                    tmp_c = 1;
                out_c += tmp_c << (7 - j);
            }
            out_string += out_c;
        }
        code = code.substr(i, code.size());
        output_file << out_string;
    }
    */
    code.clear();
    while (!input_file.eof())
    {
        input_file.get(in_char);
        unsigned char u_char = (unsigned char)in_char;
        table_it = table.find((unsigned char)u_char);
        if (table_it != table.end())
            code += table_it->second;
        else
        {
            printf("Can't find the huffman code of character %X\n", in_char);
            exit(1);
        }
        // 当总编码的长度大于预设的WRITE_BUFF_SIZE时再写入文件
        length = code.length();
        if(length > WRITE_BUFF_SIZE)
        {
            out_string.clear();
            for (i = 0; i + 7 < length; i += 8)
            {
                out_c = 0;
                for (j = 0; j < 8; ++j)
                {
                    if ('0' == code[i + j])
                        tmp_c = 0;
                    else
                        tmp_c = 1;
                    out_c += tmp_c << (7 - j);
                }
                out_string += out_c;
            }
            output_file << out_string;
            code = code.substr(i, length - i);
        }
    }

    //插入伪结束符
    //table_it = table.find(PSEUDO_EOF);
    //code += table_it->second;
    //写入不足一字节的编码及结束符，并进行尾部补齐
    length = code.length();
    out_c = 0;
    for (i = 0; i < length; ++i)
    {
        if ('0' == code[i])
            tmp_c = 0;
        else if ('1' == code[i])
            tmp_c = 1;
        out_c += tmp_c << ( 7 - (i % 8));
        if (0 == (i + 1) % 8 || i == length - 1)
        {
            output_file << out_c;
            out_c = 0;
        }
    }
    input_file.close();
    output_file.close();
    return 0;
}

int pp_encode::encode()
{
    create_pq();
    build_huffman_tree();
    calculate_huffman_codes();
    do_compress();
    return 0;
}

pp_encode::~pp_encode()
{
    //dtor
}
