// Example binary demonstrating the sam-ai compute shader.

fn main() -> anyhow::Result<()> {
    let input: Vec<u32> = vec![1, 2, 3, 4, 5, 42];
    println!("Input:  {:?}", input);

    let output = sam_ai::run_compute_example_blocking(&input)?;
    println!("Output: {:?}", output);

    // Simple check
    for (i, o) in input.iter().zip(output.iter()) {
        assert_eq!(*o, i * 2);
    }

    println!("Compute shader ran successfully.");
    Ok(())
}
