let rec print_int_list_rec list =
  match list with
  | [] -> ()
  | [last] -> print_int last
  | head :: rest -> print_int head; print_string "; "; print_int_list_rec rest

let print_int_list list =
  print_string "[";
  print_int_list_rec list;
  print_endline "]\n"

let rec sum_list_rec list sum =
  match list with
  | [] -> sum
  | head :: rest -> sum_list_rec rest sum + head

let sum_list list = sum_list_rec list 0

let get_next_line () = 
  try
    Some (read_line ())
  with
    End_of_file -> None

let rec get_next_elf_rec elves =
  match get_next_line () with
  | None | Some "" -> elves
  | Some x -> get_next_elf_rec (int_of_string x :: elves)

let get_next_elf () = get_next_elf_rec []

let rec add_up_calories_rec elf calories =
  match elf with
  | [] -> calories
  | x :: rest -> add_up_calories_rec rest calories + x

let add_up_calories elf = add_up_calories_rec elf 0

let get_next_elf_calories () =
  match get_next_elf () with
  | [] -> None
  | elf -> Some (add_up_calories elf)

let rec get_calories_list_rec list = 
  match get_next_elf_calories () with
  | None -> list
  | Some x -> get_calories_list_rec (x :: list)

let rec get_list_max_rec list max = 
  match list with
  | [] -> max
  | head :: rest -> get_list_max_rec rest (if head > max then head else max)
  
let get_list_max list =
  match list with
  | [] -> None
  | head :: rest -> Some (get_list_max_rec rest head)

let get_calories_list () = get_calories_list_rec []

let rec remove_from_list_rec list element new_list removed =
  match list with
  | [] -> new_list
  | head :: rest ->
    if ((not removed) && (head = element)) then
      remove_from_list_rec rest element new_list true
    else remove_from_list_rec rest element (head :: new_list) removed

let remove_from_list list element = remove_from_list_rec list element [] false

let rec get_maxes_from_list_rec list number output =
  match number with
  | 0 -> output
  | _ -> (
    match get_list_max list with
    | None -> raise Exit
    | Some max -> get_maxes_from_list_rec (
        remove_from_list list max
      ) (number - 1) (max :: output)
  )

let get_maxes_from_list list number =
  try
    Some (get_maxes_from_list_rec list number [])
  with
    Exit -> None

let get_max_calories calories_list number =
  match get_maxes_from_list calories_list number with
  | None -> raise Exit
  | Some x -> x
    
let () =
  try
    let max_calories_list = get_max_calories (get_calories_list ()) 3 in (
      print_string "Max calories = "; print_int_list (
        max_calories_list
      );
      print_string "Sum = ";
      print_int (sum_list max_calories_list);
      print_char '\n'
    )
  with
    Exit -> print_endline "ERROR: Invalid inputs file"
